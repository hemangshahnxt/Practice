// PatientWellnessAlertDlg.cpp : implementation file
//

// (j.gruber 2009-05-26 15:27) - PLID 34349 - created for
#include "stdafx.h"
#include <NxPracticeSharedLib/RichEditUtils.h>
#include "Practice.h"
#include "PatientWellnessAlertDlg.h"
#include "WellnessUtils.h"
#include "MultiSelectDlg.h"
#include "datetimeutils.h"
#include "InternationalUtils.h"
#include "ReportInfo.h"
#include "GlobalReportUtils.h"
#include "reports.h"
#include "WellnessDataUtils.h"


enum CompletionItemListColumns {

	cilcID = 0,
	cilcType, //TES 6/8/2009 - PLID 34505
	cilcName,
};

enum CompletionItemsColumns {

	cicID = 0,
	cicRecordID,
	cicCompletionRecordID,
	cicRecordType, //TES 6/8/2009 - PLID 34505
	cicName,
	cicStatus,
	cicValue,
	cicDate,
	cicNote,
};

enum CriteriaListColumns {

	clcID = 0,
	clcName,
	clcTypeID,
	clcTypeName,	
	clcOperator,
	clcRecordID, //TES 6/8/2009 - PLID 34510 - Renamed
	clcEmrDetailID,
	clcValue,
	clcDisplayValue,
	clcLastXDays,
	clcFulfillVal,
	clcEMRInfoType,
};

enum AvailableCriteriaListColumns {
	aclcName = 0,
	aclcTypeID = 1,
	aclcType = 2,
	aclcRecordID = 3, //TES 6/8/2009 - PLID 34510 - Renamed
	aclcEmrInfoType = 4,
};


// CPatientWellnessAlertDlg dialog

IMPLEMENT_DYNAMIC(CPatientWellnessAlertDlg, CNxDialog)

// (j.gruber 2009-06-03 12:54) - PLID 34457 - take a read only variable also
CPatientWellnessAlertDlg::CPatientWellnessAlertDlg(long nWellnessID, BOOL bIsTemplate, long nPatientID, BOOL bReadOnly, CWnd* pParent /*=NULL*/)
	: CNxDialog(CPatientWellnessAlertDlg::IDD, pParent)
{
	m_nWellnessID = nWellnessID;
	m_bIsTemplate = bIsTemplate;
	m_nPatientID = nPatientID;
	m_bHasChanged = FALSE;
	m_bReadOnly = bReadOnly;
}

CPatientWellnessAlertDlg::~CPatientWellnessAlertDlg()
{
}

void CPatientWellnessAlertDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PAT_WELL_ALERT_COMPLETE, m_chkCompleted);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_PAT_WELL_ALERT_NOTE, m_edtNotes);
	DDX_Control(pDX, IDC_PAT_WELLNESS_PREVIEW_GUIDE, m_btnGuidePreview);
	DDX_Control(pDX, IDC_PAT_WELLNESS_PREVIEW_REF, m_btnRefPreview);
	DDX_Control(pDX, IDC_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_PAT_WELL_ALERT_BKG1, m_bkg1);
	DDX_Control(pDX, IDC_PAT_WELL_ALERT_BKG2, m_bkg2);
	DDX_Control(pDX, IDC_PAT_WELL_ALERT_BKG3, m_bkg3);
	DDX_Control(pDX, IDC_PAT_WELL_ALERT_BKG4, m_bkg4);
	DDX_Control(pDX, IDC_PAT_WELL_ALERT_BKG5, m_bkg5);
}


BEGIN_MESSAGE_MAP(CPatientWellnessAlertDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CPatientWellnessAlertDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_PAT_WELLNESS_PREVIEW_GUIDE, &CPatientWellnessAlertDlg::OnBnClickedPatWellnessPreviewGuide)
	ON_BN_CLICKED(IDC_PAT_WELLNESS_PREVIEW_REF, &CPatientWellnessAlertDlg::OnBnClickedPatWellnessPreviewRef)
	ON_BN_CLICKED(IDC_CLOSE, &CPatientWellnessAlertDlg::OnBnClickedClose)
END_MESSAGE_MAP()


// CPatientWellnessAlertDlg message handlers
BOOL CPatientWellnessAlertDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnGuidePreview.AutoSet(NXB_PRINT_PREV);
		m_btnRefPreview.AutoSet(NXB_PRINT_PREV);
		m_btnClose.AutoSet(NXB_CLOSE);

		//bind the datalists
		m_pCompletionList = BindNxDataList2Ctrl(IDC_PAT_WELL_ALERT_COMPLETE_ITEMS, false);
		m_pEMRItemList = BindNxDataList2Ctrl(IDC_PAT_WELL_ALERT_EMR_ITEM_LIST, false);
		m_pCriteriaList = BindNxDataList2Ctrl(IDC_PAT_WELL_ALERT_CRITERIA, false);
		m_pAvailCriteriaList = BindNxDataList2Ctrl(IDC_PAT_WELL_AVAILABLE_CRITERIA_LIST, true);
		m_pStartDate = BindNxTimeCtrl(this, IDC_PAT_WELL_DATE_START);

		m_reGuidelines = GetDlgItem(IDC_PAT_WELL_ALERT_GUIDELINES)->GetControlUnknown();
		m_reReferenceMaterials = GetDlgItem(IDC_PAT_WELL_ALERT_REFERENCES)->GetControlUnknown();

		//taking this out for now
		NXDATALIST2Lib::IColumnSettingsPtr pCol;
		pCol = m_pCriteriaList->GetColumn(clcFulfillVal);
		pCol->PutColumnStyle(NXDATALIST2Lib::csVisible);
		pCol->StoredWidth = 0;

		//take out checkbox
		m_chkCompleted.ShowWindow(SW_HIDE);

		SetColor();

		// (j.gruber 2009-06-03 12:06) - PLID 34457 - if its read only, disable everything
		if (m_bReadOnly) {
			m_pCompletionList->Enabled = FALSE;
			m_pCriteriaList->Enabled = FALSE;
			m_pEMRItemList->Enabled = FALSE;
			m_pAvailCriteriaList->Enabled = FALSE;
			m_pStartDate->Enabled = FALSE;
			m_edtNotes.EnableWindow(FALSE);
			m_reGuidelines->Enabled = FALSE;
			m_reReferenceMaterials->Enabled = FALSE;


			//hide the OK and cancel buttons
			m_btnOK.ShowWindow(SW_HIDE);
			m_btnCancel.ShowWindow(SW_HIDE);
			m_btnClose.ShowWindow(SW_SHOW);
		}
		else {
			
			//don't need to set the other controls here because they'll be set down below

			//hide the close button
			m_btnOK.ShowWindow(SW_SHOW);
			m_btnCancel.ShowWindow(SW_SHOW);
			m_btnClose.ShowWindow(SW_HIDE);
		}


		//now load the dialog
		Load();

	}NxCatchAll("Error in  CPatientWellnessAlertDlg::OnInitDialog()");
	return TRUE;
}

void CPatientWellnessAlertDlg::SetColor() {

	try {
		if (m_bIsTemplate) {

			m_bkg1.SetColor(PREQUALIFIED_ALERT);
			m_bkg2.SetColor(PREQUALIFIED_ALERT);
			m_bkg3.SetColor(PREQUALIFIED_ALERT);
			m_bkg4.SetColor(PREQUALIFIED_ALERT);
			m_bkg5.SetColor(PREQUALIFIED_ALERT);

			m_reGuidelines->BackColor = PREQUALIFIED_ALERT;
			m_reReferenceMaterials->BackColor = PREQUALIFIED_ALERT;
		}
		else {
			if (m_bReadOnly) {
				//read onlys are completed or skipped
				m_bkg1.SetColor(COMPLETED_ALERT);
				m_bkg2.SetColor(COMPLETED_ALERT);
				m_bkg3.SetColor(COMPLETED_ALERT);
				m_bkg4.SetColor(COMPLETED_ALERT);
				m_bkg5.SetColor(COMPLETED_ALERT);

				m_reGuidelines->BackColor = COMPLETED_ALERT;
				m_reReferenceMaterials->BackColor = COMPLETED_ALERT;
			}
			else {
				m_bkg1.SetColor(PATIENT_ALERT);
				m_bkg2.SetColor(PATIENT_ALERT);
				m_bkg3.SetColor(PATIENT_ALERT);
				m_bkg4.SetColor(PATIENT_ALERT);
				m_bkg5.SetColor(PATIENT_ALERT);

				m_reGuidelines->BackColor = PATIENT_ALERT;
				m_reReferenceMaterials->BackColor = PATIENT_ALERT;
			}
		}
		
	}NxCatchAll("Error in CPatientWellnessAlertDlg::SetColor()");

}

void CPatientWellnessAlertDlg::Load() {

	try {
		if (m_bIsTemplate) {

			CString strFrom, strWhere;

			//completion items	
			//TES 6/8/2009 - PLID 34505 - Added RecordType, support for immunizations
			strFrom.Format(" (SELECT WellnessTemplateCompletionItemT.ID as ID, RecordID, RecordType, -1 as CompletionRecordID,  "
				" WellnessTemplateID, CASE WHEN RecordType = 1 THEN EMRInfoT.Name ELSE ImmunizationsT.Type END as ItemName, -1 as Status, '' as CompValue, NULL as CompDate,  "
				" '' AS Note FROM  "
				" WellnessTemplateCompletionItemT  "
				" LEFT JOIN (EmrInfoMasterT INNER JOIN EmrInfoT ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID) "
				" ON WellnessTemplateCompletionItemT.RecordID = EmrInfoMasterT.ID "
				" LEFT JOIN ImmunizationsT ON WellnessTemplateCompletionItemT.RecordID = ImmunizationsT.ID) Q"); 
			
			strWhere.Format("WellnessTemplateID = %li", m_nWellnessID);

			m_pCompletionList->FromClause = _bstr_t(strFrom);
			m_pCompletionList->WhereClause = _bstr_t(strWhere);
			
			m_pCompletionList->Requery();


			//emrinfoitems		
			//TES 6/8/2009 - PLID 34505 - Added Immunizations
			strFrom.Format("(SELECT EMRInfoMasterT.ID AS ID, convert(TINYINT,1) AS RecordType, EMRInfoT.Name "
				"FROM EmrInfoMasterT INNER JOIN EmrInfoT ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID "
				"WHERE EmrInfoMasterT.Inactive = 0 "
				"UNION SELECT ImmunizationsT.ID AS ID, convert(TINYINT,2) AS RecordType, ImmunizationsT.Type AS Name "
				"FROM ImmunizationsT) Q");			
			

			m_pEMRItemList->FromClause = _bstr_t(strFrom);			

			m_pEMRItemList->Requery();
			
			//criteria
			//TES 6/8/2009 - PLID 34510 - Added support for immunization-based criteria, renamed EmrInfoMasterID to RecordID
			//TES 6/12/2009 - PLID 34567 - Added an "Age (Years)" option.
			//TES 7/15/2009 - PLID 34510 - Made sure that "<Loading...>" doesn't show for immunization-based criteria.
			m_pCriteriaList->FromClause = "(SELECT WellnessTemplateCriterionT.ID, "
				" CASE WHEN WellnessTemplateCriterionT.Type = 1 THEN "
				"   CASE WHEN convert(int,WellnessTemplateCriterionT.Value)%12 = 0 "
				"    AND convert(int,WellnessTemplateCriterionT.Value) >= 36 "
				"   THEN 'Age (Years)' ELSE 'Age (Months)' END "
				"  WHEN WellnessTemplateCriterionT.Type = 2 THEN 'Gender' "
				"  WHEN WellnessTemplateCriterionT.Type = 4 THEN 'Active Problems' "
				"  WHEN WellnessTemplateCriterionT.Type = 5 THEN ImmunizationsT.Type "
				"  ELSE EmrInfoT.Name END AS Name, "
				" CASE WHEN WellnessTemplateCriterionT.Type IN (1,2) THEN 'Demographics' "
				"  WHEN WellnessTemplateCriterionT.Type = 3 THEN 'EMR Detail' "
				"  WHEN WellnessTemplateCriterionT.Type = 4 THEN 'EMR Problem List' "
				"  WHEN WellnessTemplateCriterionT.Type = 5 THEN 'Immunization' END AS Type,  "
				" WellnessTemplateCriterionT.Type AS TypeID,  "
				" WellnessTemplateCriterionT.RecordID, WellnessTemplateCriterionT.Operator,  "
				" WellnessTemplateCriterionT.Value,  "
				" CASE WHEN WellnessTemplateCriterionT.Type = 1 THEN "
				"  CASE WHEN IsNumeric(WellnessTemplateCriterionT.Value) = 1 THEN "
				"   CASE WHEN WellnessTemplateCriterionT.Value%12 = 0 "
				"    AND convert(int,WellnessTemplateCriterionT.Value) >= 36 "
				"   THEN convert(nvarchar(3500),WellnessTemplateCriterionT.Value/12) "
				"   ELSE WellnessTemplateCriterionT.Value END "
				"  ELSE WellnessTemplateCriterionT.Value END "
				" WHEN WellnessTemplateCriterionT.Type = 3 AND EmrInfoT.DataType = 3 THEN '<Loading...>' ELSE WellnessTemplateCriterionT.Value END AS DisplayValue, "
				" WellnessTemplateCriterionT.LastXDays, WellnessTemplateCriterionT.WellnessTemplateID,  "
				" EmrInfoT.DataType AS EmrInfoType, "
				" -1 as EMRDetailID,  '' as FulfillVal "
				" FROM WellnessTemplateCriterionT  "
				" LEFT JOIN (EmrInfoMasterT INNER JOIN EmrInfoT ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID)  "
				" ON WellnessTemplateCriterionT.RecordID = EmrInfoMasterT.ID  "
				" LEFT JOIN ImmunizationsT ON WellnessTemplateCriterionT.RecordID = ImmunizationsT.ID) "
				" AS WellnessTemplateCriteriaQ ";

			strWhere.Format("WellnessTemplateID = %li", m_nWellnessID);
			m_pCriteriaList->WhereClause = _bstr_t(strWhere);

			m_pCriteriaList->Requery();

			//disable the date and note fields
			m_pStartDate->Enabled = FALSE;

			m_edtNotes.EnableWindow(FALSE);

			//take out all but the name column
			NXDATALIST2Lib::IColumnSettingsPtr pCol;
			pCol = m_pCompletionList->GetColumn(cicStatus);
			pCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth);
			pCol->StoredWidth = 0;

			/*pCol = m_pCompletionList->GetColumn(cicValue);
			pCol->PutColumnStyle(NXDATALIST2Lib::csVisible);
			pCol->StoredWidth = 0;*/

			pCol = m_pCompletionList->GetColumn(cicDate);
			pCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth);
			pCol->StoredWidth = 0;
			
			pCol = m_pCompletionList->GetColumn(cicNote);
			pCol->PutColumnStyle(NXDATALIST2Lib::csFixedWidth);
			pCol->StoredWidth = 0;			

			pCol = m_pCompletionList->GetColumn(cicName);
			pCol->PutColumnStyle(NXDATALIST2Lib::csVisible|NXDATALIST2Lib::csWidthAuto);

			//load the guidelines and references
			ADODB::_RecordsetPtr rsTemplate = CreateParamRecordset("SELECT Name, Reference, Guideline, SpecificToPatientID "
			"FROM WellnessTemplateT WHERE ID = {INT}", m_nWellnessID);
			if (!rsTemplate->eof) {
				m_reReferenceMaterials->RichText = _bstr_t(AdoFldString(rsTemplate, "Reference"));
				m_reGuidelines->RichText = _bstr_t(AdoFldString(rsTemplate, "Guideline"));
				m_strTemplateName = AdoFldString(rsTemplate, "Name", "");
				m_nSpecificToPatientID = AdoFldLong(rsTemplate, "SpecificToPatientID", -1);
				
				CString strTemp;
				strTemp.Format("%s Wellness Alert (Prequalified) for %s", m_strTemplateName, GetExistingPatientName(m_nPatientID));
				SetWindowText(strTemp);
			}
	
		}
		else {

			CString strFrom, strWhere;

			//completion items
			//TES 6/8/2009 - PLID 34505 - Added RecordType, support for immunizations
			strFrom.Format(" (SELECT PatientWellnessCompletionItemT.ID as ID, PatientWellnessID, RecordID, RecordType, CompletionRecordID,   "
				" 	CASE WHEN RecordType = 1 THEN EMRInfoT.Name ELSE ImmunizationsT.Type END as ItemName, "
				"   CASE WHEN CompletionRecordID IS NOT NULL THEN -2 ELSE "
				"   CASE WHEN CompletionValue IS NOT NULL THEN -2 ELSE CASE WHEN CompletionDate IS NULL THEN -1 ELSE -3 END END END AS STATUS, "
				"	CompletionValue as CompValue, CompletionDate as CompDate,   "
				" 	Note FROM   "
				" 	PatientWellnessCompletionItemT   "
				" 	LEFT JOIN (EmrInfoMasterT INNER JOIN EmrInfoT ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID) "
				"   ON PatientWellnessCompletionItemT.RecordID = EmrInfoMasterT.ID "
				"   LEFT JOIN ImmunizationsT ON PatientWellnessCompletionItemT.RecordID = ImmunizationsT.ID) Q"); 
			
			strWhere.Format("PatientWellnessID = %li", m_nWellnessID);

			m_pCompletionList->FromClause = _bstr_t(strFrom);
			m_pCompletionList->WhereClause = _bstr_t(strWhere);
			
			m_pCompletionList->Requery();


			//emrinfoitems
			//TES 6/8/2009 - PLID 34505 - Added RecordType, Immunizations
			strFrom.Format("(SELECT EMRInfoMasterT.ID AS ID, convert(TINYINT,1) AS RecordType, EMRInfoT.Name "
				"FROM EmrInfoMasterT INNER JOIN EmrInfoT ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID "
				"WHERE EmrInfoMasterT.Inactive = 0 "
				"UNION SELECT ImmunizationsT.ID, convert(TINYINT,2) AS RecordType, ImmunizationsT.Type AS Name "
				"FROM ImmunizationsT) Q");			

			m_pEMRItemList->FromClause = _bstr_t(strFrom);


			m_pEMRItemList->Requery();
			
			//criteria
			//TES 6/8/2009 - PLID 34510 - Added support for immunization-based criteria, renamed EmrInfoMasterID to RecordID
			//TES 6/12/2009 - PLID 34567 - Added an "Age (Years)" option
			//TES 7/15/2009 - PLID 34510 - Made sure that "<Loading...>" doesn't show for immunization-based criteria.
			m_pCriteriaList->FromClause = "(SELECT PatientWellnessCriterionT.ID, "
				" CASE WHEN PatientWellnessCriterionT.Type = 1 THEN "
				"   CASE WHEN convert(int,PatientWellnessCriterionT.Value)%12 = 0 "
				"    AND convert(int,PatientWellnessCriterionT.Value) >= 36 "
				"   THEN 'Age (Years)' ELSE 'Age (Months)' END "
				"     WHEN PatientWellnessCriterionT.Type = 2 THEN 'Gender' "
				"     WHEN PatientWellnessCriterionT.Type = 4 THEN 'Active Problems' "
				"     WHEN PatientWellnessCriterionT.Type = 5 THEN ImmunizationsT.Type "
				"     ELSE EmrInfoT.Name END AS Name, "
				"    CASE WHEN PatientWellnessCriterionT.Type IN (1,2) THEN 'Demographics' "
				"     WHEN PatientWellnessCriterionT.Type = 3 THEN 'EMR Detail' "
				"     WHEN PatientWellnessCriterionT.Type = 4 THEN 'EMR Problem List' "
				"     WHEN PatientWellnessCriterionT.Type = 5 THEN 'Immunization' END AS Type,   "
				" 	 PatientWellnessCriterionT.Type AS TypeID,   "
				" 	 PatientWellnessCriterionT.RecordID, PatientWellnessCriterionT.Operator,   "
				" 	 PatientWellnessCriterionT.Value,   "
				" CASE WHEN PatientWellnessCriterionT.Type = 1 THEN "
				"  CASE WHEN IsNumeric(PatientWellnessCriterionT.Value) = 1 THEN "
				"   CASE WHEN PatientWellnessCriterionT.Value%12 = 0 "
				"    AND convert(int,PatientWellnessCriterionT.Value) >= 36 "
				"   THEN convert(nvarchar(3500),PatientWellnessCriterionT.Value/12) "
				"   ELSE PatientWellnessCriterionT.Value END "
				"  ELSE PatientWellnessCriterionT.Value END "
				" WHEN PatientWellnessCriterionT.Type = 3 AND EmrInfoT.DataType = 3 THEN '<Loading...>' ELSE PatientWellnessCriterionT.Value END AS DisplayValue, "
				" 	 Convert(nvarchar, PatientWellnessCriterionT.LastXDays) as LastXDays, "
				" 	 EmrInfoT.DataType AS EmrInfoType,  "
				" 	 -1 as EMRDetailID,  '' as FulfillVal, PatientWellnessID  "
				" 	 FROM PatientWellnessCriterionT   "
				" 	 LEFT JOIN (EmrInfoMasterT INNER JOIN EmrInfoT ON EmrInfoMasterT.ActiveEmrInfoID = EmrInfoT.ID)   "
				" 	 ON PatientWellnessCriterionT.RecordID = EmrInfoMasterT.ID   "
				"    LEFT JOIN ImmunizationsT ON PatientWellnessCriterionT.RecordID = ImmunizationsT.ID) "
				"	 AS Q ";

			strWhere.Format("PatientWellnessID = %li", m_nWellnessID);
			m_pCriteriaList->WhereClause = _bstr_t(strWhere);

			m_pCriteriaList->Requery();

			//load the guidelines and references
			ADODB::_RecordsetPtr rsTemplate = CreateParamRecordset("SELECT Name, Reference, Guideline, FirstPresentedDate, note, CompletedDate "
			"FROM PatientWellnessT WHERE ID = {INT}", m_nWellnessID);
			if (!rsTemplate->eof) {
				m_reReferenceMaterials->RichText = _bstr_t(AdoFldString(rsTemplate, "Reference"));
				m_reGuidelines->RichText = _bstr_t(AdoFldString(rsTemplate, "Guideline"));
				CString strName = AdoFldString(rsTemplate, "Name", "");
				//note: filling this here just for the report
				m_strTemplateName = strName;
				SetDlgItemText(IDC_PAT_WELL_ALERT_NOTE, AdoFldString(rsTemplate, "Note", ""));
				m_pStartDate->SetDateTime(AdoFldDateTime(rsTemplate, "FirstPresentedDate"));
				
				CString strTemp;
				strTemp.Format("%s Wellness Alert for %s", strName, GetExistingPatientName(m_nPatientID));
				SetWindowText(strTemp);

				COleDateTime dtNull;
				dtNull.SetDate(1800,12,31);
				m_dtComplete = AdoFldDateTime(rsTemplate, "CompletedDate", dtNull);				

			}

			m_pCriteriaList->Enabled = FALSE;
			m_pAvailCriteriaList->Enabled = FALSE;

			NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pCompletionList->GetColumn(cicNote);
			if (pCol) {
				pCol->Editable = TRUE;
			}

			pCol = m_pCompletionList->GetColumn(cicDate);
			if (pCol) {
				pCol->Editable = TRUE;
			}

			/*pCol = m_pCompletionList->GetColumn(cicValue);
			pCol->PutColumnStyle(NXDATALIST2Lib::csVisible);
			pCol->StoredWidth = 0;*/
		}
	}NxCatchAll("Error in CPatientWellnessAlertDlg::Load()");
}
BEGIN_EVENTSINK_MAP(CPatientWellnessAlertDlg, CNxDialog)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_EMR_ITEM_LIST, 16, CPatientWellnessAlertDlg::SelChosenPatWellAlertEmrItemList, VTS_DISPATCH)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_COMPLETE_ITEMS, 6, CPatientWellnessAlertDlg::RButtonDownPatWellAlertCompleteItems, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_COMPLETE_ITEMS, 10, CPatientWellnessAlertDlg::EditingFinishedPatWellAlertCompleteItems, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_COMPLETE_ITEMS, 8, CPatientWellnessAlertDlg::EditingStartingPatWellAlertCompleteItems, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_CRITERIA, 6, CPatientWellnessAlertDlg::RButtonDownPatWellAlertCriteria, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_CRITERIA, 18, CPatientWellnessAlertDlg::RequeryFinishedPatWellAlertCriteria, VTS_I2)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_CRITERIA, 19, CPatientWellnessAlertDlg::LeftClickPatWellAlertCriteria, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_GUIDELINES, 1, CPatientWellnessAlertDlg::TextChangedPatWellAlertGuidelines, VTS_NONE)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_REFERENCES, 1, CPatientWellnessAlertDlg::TextChangedPatWellAlertReferences, VTS_NONE)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_CRITERIA, 8, CPatientWellnessAlertDlg::EditingStartingPatWellAlertCriteria, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_AVAILABLE_CRITERIA_LIST, 16, CPatientWellnessAlertDlg::SelChosenPatWellAvailableCriteriaList, VTS_DISPATCH)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_CRITERIA, 10, CPatientWellnessAlertDlg::EditingFinishedPatWellAlertCriteria, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_COMPLETE_ITEMS, 18, CPatientWellnessAlertDlg::RequeryFinishedPatWellAlertCompleteItems, VTS_I2)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_COMPLETE_ITEMS, 9, CPatientWellnessAlertDlg::EditingFinishingPatWellAlertCompleteItems, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CPatientWellnessAlertDlg, IDC_PAT_WELL_ALERT_CRITERIA, 9, CPatientWellnessAlertDlg::EditingFinishingPatWellAlertCriteria, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)	
END_EVENTSINK_MAP()

void CPatientWellnessAlertDlg::SelChosenPatWellAlertEmrItemList(LPDISPATCH lpRow)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		
		//Make sure this item isn't in our list already.
		long nNewRecordID = VarLong(pRow->GetValue(cilcID));
		//TES 6/8/2009 - PLID 34505 - Check the type as well
		BYTE nNewRecordType = VarByte(pRow->GetValue(cilcType));
		NXDATALIST2Lib::IRowSettingsPtr p = m_pCompletionList->GetFirstRow();
		bool bFound = false;
		while(p && !bFound) {
			if(VarLong(p->GetValue(cicRecordID)) == nNewRecordID &&
				VarByte(p->GetValue(cicRecordType)) == nNewRecordType) {
				bFound = true;
			}
			p = p->GetNextRow();
		}

		if(bFound) {
			MsgBox("This item is already in the list of items to be completed.");
			return;
		}

		_variant_t varFalse(VARIANT_FALSE, VT_BOOL);

		//Add to our list		
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pCompletionList->GetNewRow();
		pNewRow->PutValue(cicID, (long)-1);
		pNewRow->PutValue(cicRecordID, nNewRecordID);
		pNewRow->PutValue(cicCompletionRecordID, g_cvarNull);
		pNewRow->PutValue(cicRecordType, nNewRecordType);
		pNewRow->PutValue(cicName, pRow->GetValue(cilcName));
		pNewRow->PutValue(cicStatus, (long)-1);
		pNewRow->PutValue(cicValue, g_cvarNull);
		pNewRow->PutValue(cicDate, g_cvarNull);
		pNewRow->PutValue(cicNote, _variant_t(""));
		m_pCompletionList->AddRowSorted(pNewRow, NULL);

		//reset out completed flag
		CheckCompleted();

		m_bHasChanged = TRUE;
		AddToChangedCriteriaList(lpRow);

	}NxCatchAll("Error in CPatientWellnessAlertDlg::SelChosenPatWellAlertEmrItemList");
}

//void CPatientWellnessAlertDlg::RemoveFromChangedCriteriaList(long ) 

void CPatientWellnessAlertDlg::AddToChangedCriteriaList(LPDISPATCH lpRow) 
{

	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
		
			long nID  = VarLong(pRow->GetValue(clcID));
			//make sure it doesn't exist first
			for (int i = 0; i < m_aryChangedCriteria.GetSize(); i++) {
				if (nID == m_aryChangedCriteria.GetAt(i)) {
					//it's already here, we don't need to add it
					return;
				}
			}

			//if we made it here, we didn't find it
			m_aryChangedCriteria.Add(nID);
		}
	}NxCatchAll("Error in CPatientWellnessAlertDlg::AddToChangedCriteriaList")
}

void CPatientWellnessAlertDlg::RButtonDownPatWellAlertCompleteItems(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		m_pCompletionList->CurSel = pRow;
		if(pRow == NULL) {
			return;
		}

		CMenu mnu;
		mnu.CreatePopupMenu();
		mnu.AppendMenu(MF_ENABLED, 1, "Remove");
		CPoint ptClicked(x, y);
		GetDlgItem(IDC_PAT_WELL_ALERT_COMPLETE_ITEMS)->ClientToScreen(&ptClicked);
		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);
		if(nResult == 1) {
			if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to permanently remove this completion item? ")) {
				return;
			}
	
			//get the ID of the row
			long nCompletionID = VarLong(pRow->GetValue(cicID));
			long nRecordID = VarLong(pRow->GetValue(cicRecordID), -1);
			//TES 6/8/2009 - PLID 34505 - Pull the type as well
			BYTE nRecordType = VarByte(pRow->GetValue(cicRecordType), -1);

			if (nCompletionID != -1) {
				//add it to our deletion array
				m_aryDeletedCompletionItems.Add(nCompletionID);
				if (nRecordID != -1) {
					m_aryCompletionRecordIDs.Add(nRecordID);
					m_aryCompletionRecordTypes.Add(nRecordType);
				}
				else {
					//this shouldn't happen
					ASSERT(FALSE);
				}
			}
			m_pCompletionList->RemoveRow(pRow);
			m_bHasChanged = TRUE;

			CheckCompleted();
			
		}
		

	}NxCatchAll("Error in CPatientWellnessAlertDlg::RButtonDownPatWellAlertCompleteItems");
}

void CPatientWellnessAlertDlg::CheckCompleted() {

	try {
		if (m_bIsTemplate) {
			return;
		}

		if (m_pCompletionList->GetRowCount() == 0) {
			m_bComplete = FALSE;
			return;
		}

		//run through the completed list and see if everything is marked off
		long nCountComplete = 0;
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pCompletionList->GetFirstRow();
		while (pRow) {
			long nIsCompleted = VarLong(pRow->GetValue(cicStatus));
			//count it if it is completed or skipped
			if (nIsCompleted != -1) {
				nCountComplete++;
			}
			pRow = pRow->GetNextRow();
		}

		COleDateTime dtNull;
		dtNull.SetDate(1800,12,31);

		if (nCountComplete == m_pCompletionList->GetRowCount()) {
			//everything is completed, so check the box
			//m_chkCompleted.SetCheck(1);

			//and set our boolean
			m_bComplete = TRUE;

			//if we don't have a date yet then set it
			//if (m_dtCompleted == dtNull) {
				//m_dtCompleteed = COleDateTime::GetCurrentTime();
			//}
		}
		else {
			//everythign isn't complete, so set our settings
			//m_chkCompleted.SetCheck(0);

			m_bComplete = FALSE;

			m_dtComplete = dtNull;
		}
	}NxCatchAll("Error in CPatientWellnessAlertDlg::CheckCompleted()");
}

void CPatientWellnessAlertDlg::EditingFinishedPatWellAlertCompleteItems(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		if (bCommit) {

			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if (pRow) {

				switch (nCol) {

					case cicStatus:
						{
							//if they checked completed, we need to add the date, if they unchecked it, we need to remove the date
							long nStatus = VarLong(pRow->GetValue(cicStatus));

							COleDateTime dtNow = COleDateTime::GetCurrentTime();
							COleDateTime dtTemp;
							dtTemp.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());
							_variant_t vardt;
							vardt.vt = VT_DATE;
							vardt.date = dtTemp;	

							switch (nStatus) {

								case -1:
									//changed it from something to nothing
									pRow->PutValue(cicDate, g_cvarNull);
									pRow->PutValue(cicValue, g_cvarNull);
								break;

								case -2:
									//they manually completed it					
									pRow->PutValue(cicDate, vardt);
									pRow->PutValue(cicValue, _variant_t(""));
								break;

								case -3:
									//they skipped it
									pRow->PutValue(cicDate, vardt);
									pRow->PutValue(cicValue, g_cvarNull);
								break;
							}				
							CheckCompleted();
						}
					break;

					case cicDate:
					break;

					case cicNote:
					break;
				}

			}

		}
	}NxCatchAll("Error in CPatientWellnessAlertDlg::EditingFinishedPatWellAlertCompleteItems");
}

void CPatientWellnessAlertDlg::EditingStartingPatWellAlertCompleteItems(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		if (m_bIsTemplate) {

			//they can't edit this!
			*pbContinue = FALSE;
			return;
		}
		else {

			//check to see if they are trying to edit the completion date without it being completed
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (pRow) {
			
				long nStatus = VarLong(pRow->GetValue(cicStatus));
				switch (nCol) {

					case cicStatus:

						//check to see what the status is currently
						
						if (nStatus == -2) {
							//its completed, so we need to check what kind of completed
							CString strCompValue = VarString(pRow->GetValue(cicValue), "");
							if (!strCompValue.IsEmpty()) {
								//its a auto complete, don't let them change it
								MsgBox("This item has been auto-completed, you may not change it's status");
								*pbContinue = FALSE;							
							}	
						}							

					break;				

					case cicDate:									
						
						if (nStatus == -1) {
							//it's not completed or skipped yet, don't let them edit the date
							*pbContinue = FALSE;
						}
						else if (nStatus == -2) {
							//its completed, so we need to check what kind of completed
							CString strCompValue = VarString(pRow->GetValue(cicValue), "");
							if (!strCompValue.IsEmpty()) {
								//its a auto complete, don't let them change it
								MsgBox("This item has been auto-completed, you may not change the date.");
								*pbContinue = FALSE;
							}	
						}							
					break;

					case cicNote:
						if (nStatus == -2) {
							//its completed, so we need to check what kind of completed
							CString strCompValue = VarString(pRow->GetValue(cicValue), "");
							if (!strCompValue.IsEmpty()) {
								//its a auto complete, don't let them change it
								MsgBox("This item has been auto-completed, you may not edit it.");
								*pbContinue = FALSE;
							}	
						}		
					break;
				}
			}
		}

	}NxCatchAll("Error in CPatientWellnessAlertDlg::EditingStartingPatWellAlertCompleteItems");
}

void CPatientWellnessAlertDlg::RButtonDownPatWellAlertCriteria(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {

		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		m_pCriteriaList->CurSel = pRow;
		if(pRow == NULL) {
			return;
		}

		CMenu mnu;
		mnu.CreatePopupMenu();		
		mnu.AppendMenu(MF_ENABLED, 1, "Remove");
		CPoint ptClicked(x, y);
		GetDlgItem(IDC_PAT_WELL_ALERT_CRITERIA)->ClientToScreen(&ptClicked);
		int nResult = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptClicked.x, ptClicked.y, this);
		if(nResult == 1) {
			if(IDYES != MsgBox(MB_YESNO, "Are you sure you wish to permanently remove this criteria? ")) {
				return;
			}
			long nID = VarLong(pRow->GetValue(clcID), -1);
			if (nID != -1) {
				m_aryDeletedCriteria.Add(nID);
			}
			m_pCriteriaList->RemoveRow(pRow);
			m_bHasChanged = TRUE;
		}		

	}NxCatchAll("Error in CPatientWellnessAlertDlg::RButtonDownPatWellAlertCriteria");
}

void CPatientWellnessAlertDlg::RequeryFinishedPatWellAlertCriteria(short nFlags)
{
	try {

		// go through and set the appropriate operators to be available for each row.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCriteriaList->GetFirstRow();
		while(pRow) {
			WellnessTemplateCriterionType wtct = (WellnessTemplateCriterionType)VarShort(pRow->GetValue(clcTypeID));
			EmrInfoType eit = (EmrInfoType)VarShort(pRow->GetValue(clcEMRInfoType),-1);
			//TES 6/8/2009 - PLID 34510 - Renamed EmrInfoMasterID to RecordID
			long nRecordID = VarLong(pRow->GetValue(clcRecordID),-1);			
			WellnessTemplateCriteriaOperator wtco = (WellnessTemplateCriteriaOperator)VarByte(pRow->GetValue(clcOperator));
			pRow->PutRefCellFormatOverride(clcOperator, GetOperatorFormatSettings(wtct));
			//Configure the Value field appropriately.
			//TES 6/2/2009 - PLID 34302 - Added a parameter for the operator.
			pRow->PutRefCellFormatOverride(clcDisplayValue, GetValueFormatSettings(wtct, eit, nRecordID, (WellnessTemplateCriteriaOperator)VarByte(pRow->GetValue(clcOperator))));

			//if this is a multi-select list, load the data elements.
			//TES 6/8/2009 - PLID 34510 - Make sure this is actually an EMR-Item-based criterion.
			if(wtct == wtctEmrItem && eit == eitMultiList) {
				if(wtco == wtcoFilledIn || wtco == wtcoNotFilledIn || wtco == wtcoExists || wtco == wtcoDoesNotExist) {
					pRow->PutValue(clcDisplayValue, g_cvarNull);
				}
				else {
					CString strValue = VarString(pRow->GetValue(clcValue),"");
					strValue.TrimLeft(" ");
					if(strValue.IsEmpty()) {
						pRow->PutValue(clcDisplayValue, _bstr_t("<Select...>"));
					}
					else {
						// Make sure the value is valid (needs to be space-delimited list of EmrDataGroupIDs).
						if(strValue.SpanIncluding("1234567890 ").GetLength() != strValue.GetLength()) {
							//This is an invalid value
							ASSERT(FALSE);
							pRow->PutValue(clcValue, _bstr_t(""));
							pRow->PutValue(clcDisplayValue, _bstr_t("<Select...>"));
						}
						else {
							//We've got the IDs, now load the names.
							CString strDataGroupIDs;
							int nSpace = strValue.Find(" ");
							while(nSpace != -1) {
								strDataGroupIDs += strValue.Left(nSpace) + ",";
								strValue = strValue.Mid(nSpace+1);
								nSpace = strValue.Find(" ");
							}
							strDataGroupIDs += strValue;
							ADODB::_RecordsetPtr rsData = CreateRecordset("SELECT Data FROM EmrDataT WHERE EmrDataGroupID IN (%s) "
								"AND EmrInfoID = (SELECT ActiveEmrInfoID FROM EmrInfoMasterT WHERE ID = %li)", 
								_Q(strDataGroupIDs), nRecordID);
							CString strDisplayValue;
							while(!rsData->eof) {
								strDisplayValue += AdoFldString(rsData, "Data") + ", ";
								rsData->MoveNext();
							}
							if(strDisplayValue.IsEmpty()) {
								pRow->PutValue(clcDisplayValue, _bstr_t("<Select...>"));
							}
							else {
								strDisplayValue = strDisplayValue.Left(strDisplayValue.GetLength()-2);
								pRow->PutValue(clcDisplayValue, _bstr_t(strDisplayValue));
							}
						}
					}
				}
			}

			//add it to our initial criteria list
			m_aryInitialCriteria.Add(pRow->GetValue(clcID));

			pRow = pRow->GetNextRow();
		}

	}NxCatchAll("Error in CPatientWellnessAlertDlg::RequeryFinishedPatWellAlertCriteria");
}

void CPatientWellnessAlertDlg::LeftClickPatWellAlertCriteria(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol) {
			case clcDisplayValue:
				{
					WellnessTemplateCriterionType wtct = (WellnessTemplateCriterionType)VarByte(pRow->GetValue(clcTypeID));
					EmrInfoType eit = (EmrInfoType)VarByte(pRow->GetValue(clcEMRInfoType),-1);
					WellnessTemplateCriteriaOperator wtco = (WellnessTemplateCriteriaOperator)VarByte(pRow->GetValue(clcOperator));
					if(wtco == wtcoFilledIn || wtco == wtcoNotFilledIn || wtco == wtcoExists || wtco == wtcoDoesNotExist) {
						//TES 6/2/2009 - PLID 34302 - They can't change the value for this operator.
						return;
					}
					//TES 6/8/2009 - PLID 34510 - Make sure this is actually an EMR-Item-based criterion.
					if(wtct == wtctEmrItem && eit == eitMultiList) {
						//We're going to pop up a dialog.
						//TES 6/8/2009 - PLID 34510 - Renamed EmrInfoMasterID to RecordID
						long nRecordID = VarLong(pRow->GetValue(clcRecordID),-1);
						CString strValue = VarString(pRow->GetValue(clcValue),"");
						WellnessTemplateCriteriaOperator wtco = (WellnessTemplateCriteriaOperator)VarByte(pRow->GetValue(clcOperator));
						//Load the pre-selected values.
						CArray<long,long> arDataGroupIDs;
						int nSpace = strValue.Find(" ");
						while(nSpace != -1) {
							arDataGroupIDs.Add(atol(strValue.Left(nSpace)));
							strValue = strValue.Mid(nSpace+1);
							nSpace = strValue.Find(" ");
						}
						arDataGroupIDs.Add(atol(strValue));
						//Prepare the dialog, force only one selection unless the operator
						// is Equal or Not Equal
						// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
						CMultiSelectDlg dlg(this, "EmrDataT");
						dlg.PreSelect(arDataGroupIDs);
						unsigned long nMaxSelections = 1;
						CString strValueLabel = "value";
						if(wtco == wtcoEqual || wtco == wtcoNotEqual) {
							nMaxSelections = 0xFFFFFFFF;
							strValueLabel = "values";
						}
						if(IDOK == dlg.Open("EmrDataT", 
							// (j.gruber 2009-07-02 16:27) - PLID 34350 - take out inactives and labels
							// (j.jones 2009-07-15 17:56) - PLID 34916 - ensure we filter on list items only, incase any table columns exist
							FormatString("EMRDataT.Inactive = 0 AND EMRDataT.IsLabel = 0 "
								"AND EMRDataT.ListType = 1 "
								"AND EmrInfoID = (SELECT ActiveEmrInfoID FROM EmrInfoMasterT WHERE ID = %li)", nRecordID),
							"EmrDataGroupID", "Data", 
							"Please select the " + strValueLabel + " you wish this criteria to use for this Wellness Template.",
							0, nMaxSelections)) {
							// they've selected some new values
							dlg.FillArrayWithIDs(arDataGroupIDs);
							CString strNewValue;
							for(int i = 0; i < arDataGroupIDs.GetSize(); i++) {
								strNewValue += AsString(arDataGroupIDs[i]) + " ";
							}
							strNewValue.TrimRight(" ");							
							//Update the "real" value field.
							pRow->PutValue(clcValue, _bstr_t(strNewValue));
							//now update the display value field with the names of the selected items.
							CVariantArray vaNewNames;
							dlg.FillArrayWithNames(vaNewNames);
							CString strNewNames;
							for(i = 0; i < vaNewNames.GetSize(); i++) {
								strNewNames += AsString(vaNewNames[i]) + "; ";
							}
							if(strNewNames.IsEmpty()) {
								pRow->PutValue(clcDisplayValue, _bstr_t("<Select...>"));
							}
							else {
								strNewNames = strNewNames.Left(strNewNames.GetLength() - 2);
								pRow->PutValue(clcDisplayValue, _bstr_t(strNewNames));
							}
							m_bHasChanged = TRUE;
						}
						
					}
				}
				break;
		}

	}NxCatchAll("Error in CPatientWellnessAlertDlg::LeftClickPatWellAlertCriteria ");
}

void CPatientWellnessAlertDlg::TextChangedPatWellAlertGuidelines()
{
	m_bHasChanged = TRUE;
}

void CPatientWellnessAlertDlg::TextChangedPatWellAlertReferences()
{
	m_bHasChanged = TRUE;
}

void CPatientWellnessAlertDlg::EditingStartingPatWellAlertCriteria(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		switch(nCol) {
			case clcDisplayValue:
				{
					//They can't set the value if the operator is Filled In or Not Filled In
					WellnessTemplateCriteriaOperator wtco = (WellnessTemplateCriteriaOperator)VarShort(pRow->GetValue(clcOperator));
					if(wtco == wtcoFilledIn || wtco == wtcoNotFilledIn || wtco == wtcoExists || wtco == wtcoDoesNotExist) {
						*pbContinue = false;
					}
				}
				break;
			case clcLastXDays:
				{
					//They can't set the Last X Days if the criteria is Age, Gender, or Problem List
					WellnessTemplateCriterionType wtct = (WellnessTemplateCriterionType)VarShort(pRow->GetValue(clcTypeID));
					// (j.fouts 2013-10-07 10:15) - PLID 56237 - Intiail Commit for CDS
					if(wtct == wtctAge || wtct == wtctGender || wtct == wtctEmrProblemList || wtct == wtctFilter) {
						*pbContinue = false;
					}
				}
				break;
		}
		//Check to see if this criteria is completed and if so, make sure they know they are changing it and it will be uncompleted
		/*NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {

			CString strFulfillValue = VarString(pRow->GetValue(clcFulfillVal), "");
			if (!strFulfillValue.IsEmpty()) {
				if (MsgBox(MB_YESNO, "This criteria has already been completed.  Changing any value could potentially make it uncomplete.\nAre you sure you want to continue?") == IDNO) {
					*pbContinue = FALSE;
				}
			}
		}*/			

	}NxCatchAll("CPatientWellnessAlertDlg::EditingStartingPatWellAlertCriteria");
}

void CPatientWellnessAlertDlg::SelChosenPatWellAvailableCriteriaList(LPDISPATCH lpRow)
{
	try {
		
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		
		if(pRow == NULL) {
			return;
		}

		short nNewType = (short)VarByte(pRow->GetValue(aclcTypeID));
		//TES 6/8/2009 - PLID 34510 - Renamed EmrInfoMasterID to RecordID
		_variant_t varNewRecordID = pRow->GetValue(aclcRecordID);
		//Set the operator to the default for this type.
		WellnessTemplateCriteriaOperator wtco = GetDefaultOperator((WellnessTemplateCriterionType)nNewType);

		//Add to our list
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pCriteriaList->GetNewRow();
		pNewRow->PutValue(clcID, (long)-1);
		pNewRow->PutValue(clcName, pRow->GetValue(aclcName));
		pNewRow->PutValue(clcTypeName, pRow->GetValue(aclcType));
		pNewRow->PutValue(clcTypeID, nNewType);
		pNewRow->PutValue(clcRecordID, varNewRecordID);
		// Now, set the available operators according to type.
		pNewRow->PutRefCellFormatOverride(clcOperator, GetOperatorFormatSettings((WellnessTemplateCriterionType)nNewType));
		//Likewise, set up the Value field based on the type (and EMR Item, if any) of this row.
		//TES 6/2/2009 - PLID 34302 - Added a parameter for the operator.
		pNewRow->PutRefCellFormatOverride(clcDisplayValue, GetValueFormatSettings((WellnessTemplateCriterionType)nNewType, (EmrInfoType)VarByte(pRow->GetValue(aclcEmrInfoType),-1), VarLong(varNewRecordID,-1), wtco));		
		pNewRow->PutValue(clcOperator, (short)wtco);
		pNewRow->PutValue(clcValue, g_cvarNull);
		EmrInfoType eit = (EmrInfoType)VarByte(pRow->GetValue(aclcEmrInfoType),-1);
		//don't say select if the can't select 
		if((WellnessTemplateCriterionType)nNewType == wtctEmrItem && eit == eitMultiList && (wtco != wtcoFilledIn && wtco != wtcoNotFilledIn && wtco != wtcoExists && wtco != wtcoDoesNotExist)) {
			//Let them know they need to click on this row to change it.
			pNewRow->PutValue(clcDisplayValue, _bstr_t("<Select...>"));
		}
		else {
			pNewRow->PutValue(clcDisplayValue, g_cvarNull);
		}
		pNewRow->PutValue(clcLastXDays, g_cvarNull);
		pNewRow->PutValue(clcEMRInfoType, pRow->GetValue(aclcEmrInfoType));
		m_pCriteriaList->AddRowSorted(pNewRow, NULL);
		m_pCriteriaList->CurSel = pNewRow;
		GetDlgItem(IDC_PAT_WELL_ALERT_CRITERIA)->SetFocus();
		//Start them editing the row.
		m_pCriteriaList->StartEditing(pNewRow, clcOperator);

		m_bHasChanged = TRUE;

	}NxCatchAll("Error in CPatientWellnessAlertDlg::SelChosenPatWellAvailableCriteriaList");
}

void CPatientWellnessAlertDlg::OnBnClickedOk()
{
	try {

		if (!Save()) {
			return;
		}
		OnOK();

	}NxCatchAll("Error in CPatientWellnessAlertDlg::OnBnClickedOk()");
}

BOOL CPatientWellnessAlertDlg::GenerateCriteriaSaveString(CString &strSqlBatch) {

	try {

		//only need to delete if we are specific to a patient already
		if (m_nSpecificToPatientID != -1) {
			//delete all the criteria that was removed
			if (m_aryDeletedCriteria.GetSize() > 0) {
				CString strExistingIDs = "(";
				for (int i = 0; i < m_aryDeletedCriteria.GetSize(); i++) {
					long nCompletionID = m_aryDeletedCriteria[i];
					
					if (nCompletionID != -1) {
						strExistingIDs += AsString(nCompletionID) + ",";
					}			
				}

				//take off the trailing , and add a )
				strExistingIDs.TrimRight(",");
				strExistingIDs += ")";
				if (strExistingIDs != "()") {
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM WellnessPatientQualificationT WHERE WellnessTemplateCriterionID IN %s AND PatientID = %li", strExistingIDs, m_nPatientID);
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM WellnessTemplateCriterionT WHERE ID IN %s", strExistingIDs);
				}
			}
		}

		//loop through the criteria and generate the values
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCriteriaList->GetFirstRow();
		while (pRow) {

			//now loop through the list generating the values		
			long nOperator = (long)VarShort(pRow->GetValue(clcOperator));
			BOOL bValueAlreadySet = FALSE;
			CString strValue;
			WellnessTemplateCriteriaOperator wtco = (WellnessTemplateCriteriaOperator)nOperator;
			//TES 5/29/2009 - PLID 34302 - The value is also not used for Exists/Does Not Exist.
			if(wtco == wtcoFilledIn || wtco == wtcoNotFilledIn || wtco == wtcoExists || wtco == wtcoDoesNotExist) {
				// The value is not used with this operator, so clear it out.
				strValue = "";
				bValueAlreadySet = TRUE;
			}
			else if(wtco != wtcoEqual && wtco != wtcoNotEqual) {
				EmrInfoType eit = (EmrInfoType)VarShort(pRow->GetValue(clcEMRInfoType),-1);
				WellnessTemplateCriterionType wtct = (WellnessTemplateCriterionType)VarShort(pRow->GetValue(clcTypeID));
				//TES 6/8/2009 - PLID 34510 - Make sure this is actually an EMR-Item-based criterion.
				if(wtct == wtctEmrItem && eit == eitMultiList) {
					//With any operator other than equal or not equal, they
					// can only have one selected item in the Value field, so truncate the list to one entry.
					CString strCurrentValue = VarString(pRow->GetValue(clcValue),"");
					int nSpace = strCurrentValue.Find(" ");
					if(nSpace != -1) {
						strValue = strCurrentValue.Left(nSpace);
						bValueAlreadySet = TRUE;
					}
				}
			}

			if (!bValueAlreadySet) {

				strValue = VarString(pRow->GetValue(clcValue), "");
			}

			long nLastXDays;
			CString strLastXDays;

			nLastXDays = VarLong(pRow->GetValue(clcLastXDays), -1);
			if (nLastXDays == -1) {
				strLastXDays = "NULL";
			}
			else {
				strLastXDays.Format("%li", nLastXDays);
			}

			//TES 6/8/2009 - PLID 34510 - Renamed EmrInfoMasterID to RecordID
			long nRecordID = VarLong(pRow->GetValue(clcRecordID), -1);
			CString strRecordID;
			if (nRecordID == -1) {
				strRecordID = "NULL";
			}
			else {
				strRecordID.Format("%li", nRecordID);
			}
			long nTypeID = (long)VarShort(pRow->GetValue(clcTypeID));
					
			if (m_nSpecificToPatientID == -1) {				
				
				//its a new row 
				//TES 6/8/2009 - PLID 34510 - Renamed EmrInfoMasterID to RecordID
				AddStatementToSqlBatch(strSqlBatch, 
					" INSERT INTO WellnessTemplateCriterionT "
					" (WellnessTemplateID, Type, Operator, RecordID, Value, LastXDays) "
					" VALUES (@nWellnessID, %li, %li, %s, '%s', %s) ",
					nTypeID, nOperator, strRecordID, _Q(strValue), strLastXDays);
			}
			else {

				long nCriteriaID = VarLong(pRow->GetValue(clcID), -1);

				if (nCriteriaID == -1) {

					//its a new row 
					//TES 6/8/2009 - PLID 34510 - Renamed EmrInfoMasterID to RecordID
					AddStatementToSqlBatch(strSqlBatch, 
						" INSERT INTO WellnessTemplateCriterionT "
						" (WellnessTemplateID, Type, Operator, RecordID, Value, LastXDays) "
						" VALUES (%li, %li, %li, %s, '%s', %s) ",
						m_nWellnessID, nTypeID, nOperator, strRecordID, _Q(strValue), strLastXDays);
				}
				else {
					
					//it need to be updated
					//TES 6/8/2009 - PLID 34510 - Renamed EmrInfoMasterID to RecordID
					AddStatementToSqlBatch(strSqlBatch, 
						" UPDATE WellnessTemplateCriterionT SET "
						" Type = %li, Operator = %li, RecordID = %s, "
						" Value = '%s', LastXDays = %s WHERE ID = %li ",
						nTypeID, nOperator, strRecordID, _Q(strValue), strLastXDays, nCriteriaID);
				}
			}

			pRow = pRow->GetNextRow();

		}	


		return TRUE;
	}NxCatchAll("Error in CPatientWellnessAlertDlg::GenerateCriteriaSaveString");

	return FALSE;
}

BOOL CPatientWellnessAlertDlg::GenerateTemplateCompletionSaveString(CString &strSqlBatch) {

	try {

		//see if we are on a patient template
		if (m_nSpecificToPatientID != -1) {

			//first loop and get a list of ids that are in there, for deletion
			if (m_aryDeletedCompletionItems.GetSize() > 0) {
				CString strExistingIDs = "(";
				for (int i = 0; i < m_aryDeletedCompletionItems.GetSize(); i++) {
					long nCompletionID = m_aryDeletedCompletionItems[i];
					
					if (nCompletionID != -1) {
						strExistingIDs += AsString(nCompletionID) + ",";
					}			
				}

				//take off the trailing , and add a )
				strExistingIDs.TrimRight(",");
				strExistingIDs += ")";
				if (strExistingIDs != "()") {
				
					//we have to delete from the prequalification data also
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM WellnessTemplateCompletionItemT WHERE ID IN %s", strExistingIDs);
				}

			}
		}	
		
		
		//now loop to save what is still there
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pCompletionList->GetFirstRow();
		while (pRow) {

			long nCompletionID = VarLong(pRow->GetValue(cicID), -1);
			long nRecordID = VarLong(pRow->GetValue(cicRecordID));
			//TES 6/8/2009 - PLID 34505 - Added RecordType
			BYTE nRecordType = VarByte(pRow->GetValue(cicRecordType));

			if (m_nSpecificToPatientID == -1) {
				
				//add new
				AddStatementToSqlBatch(strSqlBatch, 
					" INSERT INTO WellnessTemplateCompletionItemT "
					" (RecordID, RecordType, WellnessTemplateID) "
					" VALUES (%li, %i, @nWellnessID) ",
					nRecordID, nRecordType);	
			}
			else {
				
				//they can't change anything on the rows, so we don't need to save it if it already exists
				if (nCompletionID == -1) {				
					//its a new row 
					AddStatementToSqlBatch(strSqlBatch, 
						" INSERT INTO WellnessTemplateCompletionItemT "
						" (RecordID, RecordType, WellnessTemplateID) "
						" VALUES (%li, %i, %li) ",
						nRecordID, nRecordType, m_nWellnessID);	
				}				
			}
			pRow = pRow->GetNextRow();
		}
				
		return TRUE;
	}NxCatchAll("Error In CPatientWellnessAlertDlg::GenerateTemplateCompletionSaveString");

	return FALSE;

}

BOOL CPatientWellnessAlertDlg::SaveTemplate() {

	try {			
		
		//update
		CString strSqlBatch;			
		strSqlBatch = BeginSqlBatch();

		CString strGuideline = (LPCTSTR)m_reGuidelines->RichText;
		CString strReference = (LPCTSTR)m_reReferenceMaterials->RichText;				

		if (m_nSpecificToPatientID == -1) {

			//see if anything changed
			if (!m_bHasChanged) {
				//we don't want to save it as a patient template because they didn't change anything
				//let's do nothing
				if (IDYES == MsgBox(MB_YESNO, "No changes have been made to this template, please make a change in order to save it as a patient specific template.\nWould you like to continue without saving?")) {
					return TRUE;
				}
				else {
					return FALSE;
				}
			}

			//adding
			AddDeclarationToSqlBatch(strSqlBatch, "DECLARE @nWellnessID INT;");
			AddStatementToSqlBatch(strSqlBatch, "SET NOCOUNT ON;");
			
			AddStatementToSqlBatch(strSqlBatch, 
				" INSERT INTO WellnessTemplateT (Name, Guideline, Reference, SpecificToPatientID, OriginalWellnessTemplateID) "
				" VALUES ('%s', '%s', '%s', %li, %li)",
				_Q(m_strTemplateName), _Q(strGuideline), _Q(strReference), 
				m_nPatientID, m_nWellnessID);

			AddStatementToSqlBatch(strSqlBatch, "SET @nWellnessID = @@IDENTITY;");	

			//since we are getting rid of the general template for this patient, we need to remove any prequalification data that went along with it
			AddStatementToSqlBatch(strSqlBatch, "DELETE FROM WellnessPatientQualificationT WHERE PatientID = %li AND WellnessTemplateCriterionID IN "
				" (SELECT ID FROM WellnessTemplateCriterionT WHERE WellnessTemplateID = %li)", m_nPatientID, m_nWellnessID);
		}
		else {

			//updating only				
			AddStatementToSqlBatch(strSqlBatch, "SET NOCOUNT ON;");

			AddStatementToSqlBatch(strSqlBatch, 
				" UPDATE WellnessTemplateT SET Guideline = '%s', Reference = '%s' "
				" WHERE ID = %li",
				_Q(strGuideline), _Q(strReference), m_nWellnessID);
		}	

		//now do the completion items
		if (!GenerateTemplateCompletionSaveString(strSqlBatch)) {
			return FALSE;
		}

		//criteria time!!
		if (!GenerateCriteriaSaveString(strSqlBatch)) {
			return FALSE;
		}				

		//get the criteria for this wellnesstemplateID
		AddStatementToSqlBatch(strSqlBatch, "SET NOCOUNT OFF;");

		if (m_nSpecificToPatientID == -1) {

			//we added new ones
			AddStatementToSqlBatch(strSqlBatch, "SELECT ID FROM WellnessTemplateCriterionT WHERE WellnessTemplateID = @nWellnessID");
		}
		else {
			//it's existing
			AddStatementToSqlBatch(strSqlBatch, "SELECT ID FROM WellnessTemplateCriterionT WHERE WellnessTemplateID = %li", m_nWellnessID);
		}			

		ADODB::_RecordsetPtr rsCriteria = CreateRecordsetStd(strSqlBatch);

		CDWordArray aryCriteria;
		while (!rsCriteria->eof) {
			aryCriteria.Add(AdoFldLong(rsCriteria, "ID"));
			rsCriteria->MoveNext();
		}

		//now we have to get just the values that changed or are new
		if (m_nSpecificToPatientID == -1) {
			//they are all new, we have nothing to match up
		}
		else {
			GetChangedCriteriaOnly(&aryCriteria);
		}

		//now call the criteria function	
		UpdatePatientWellnessQualification_TemplateCriteria(GetRemoteData(), aryCriteria);

		return TRUE;
	}NxCatchAll("Error in CPatientWellnessAlertDlg::SaveTemplate()");

	return FALSE;
}


void CPatientWellnessAlertDlg::GetChangedCriteriaOnly(CDWordArray *pCriteriaArray) {

	CDWordArray aryTemp;
	
	//loop through our array and see if they were in our initial list or are in the changed list
	for (int i = 0; i < pCriteriaArray->GetSize(); i++) {

		long nID = pCriteriaArray->GetAt(i);

		//see if it is in our changed array
		BOOL bInChangedArray = FALSE;
		for (int j = 0; j < m_aryChangedCriteria.GetSize(); j++) {
			if (m_aryChangedCriteria.GetAt(j) == nID) {
				bInChangedArray = TRUE;
			}
		}

		if (bInChangedArray) {
			//it was changed when they had the alert open, so add it to the list
			aryTemp.Add(nID);
		}
		else {

			//we didn't find it in the temp array, was it in our initial array?
			BOOL bInInitialArray = FALSE;
			for (int j = 0; j < m_aryInitialCriteria.GetSize(); j++) {
				if (m_aryInitialCriteria.GetAt(j) == nID) {
					bInInitialArray = TRUE;
				}
			}

			if (bInInitialArray) {
				//it was in our initial array, but not in our changed array so make sure its not in our deleted array, in case it got deleted and readded
				BOOL bInDeletedArray = FALSE;
				for (int j = 0; j < m_aryDeletedCriteria.GetSize(); j++) {
					if (m_aryDeletedCriteria.GetAt(j) == nID) {
						bInDeletedArray = TRUE;
					}
				}

				if (bInDeletedArray) {
					//it was deleted, but we still got it back from the database, so it must be a new record with the same ID, add it to our array
					aryTemp.Add(nID);
				}
				else {
					//it wasn't deleted either, so it must have never changed, don't add it
				}
			}
			else {
				//it's not in our initial array, so it must've been added, so add it to our array
				aryTemp.Add(nID);
			}
		}
	}

	//now copy all the items from the temp arrary into the one we passed in
	pCriteriaArray->RemoveAll();

	for (int i = 0; i < aryTemp.GetSize(); i++) {
		pCriteriaArray->Add(aryTemp.GetAt(i));
	}
}

BOOL CPatientWellnessAlertDlg::Save() {

	try {

		// (j.gruber 2009-06-03 12:27) - PLID 34457 - make sure we don't save read onlys
		if (m_bReadOnly) {
			return TRUE;
		}		

		CWaitCursor cwait;

		if (m_bIsTemplate) {

			//first validate
			if (ValidateCurrentTemplate()) {

				return SaveTemplate();			
			}
			else {
				return FALSE;
			}
		}
		else {

			//validate the date
			if (m_pStartDate->GetStatus() != 1) {
				MsgBox("Please enter a valid start date.");
				return FALSE;
			}


			COleDateTime dt = m_pStartDate->GetDateTime();
			if(dt.GetStatus() != COleDateTime::valid || dt.GetYear() <= 1899) {
				MsgBox("Please enter a valid start date.");
				return FALSE;
			}


			CString strSqlBatch;			
			strSqlBatch = BeginSqlBatch();

			CString strGuideline = (LPCTSTR)m_reGuidelines->RichText;
			CString strReference = (LPCTSTR)m_reReferenceMaterials->RichText;
			CString strNote;
			GetDlgItemText(IDC_PAT_WELL_ALERT_NOTE, strNote);
			
			COleDateTime dtFirstPresented = m_pStartDate->GetDateTime();						
			
			//first loop and get a list of ids that are in there, for deletion
			if (m_aryDeletedCompletionItems.GetSize() > 0) {
				CString strExistingIDs = "(";
				for (int i = 0; i < m_aryDeletedCompletionItems.GetSize(); i++) {
					long nCompletionID = m_aryDeletedCompletionItems[i];
					
					if (nCompletionID != -1) {
						strExistingIDs += AsString(nCompletionID) + ",";
					}			
				}

				//take off the trailing , and add a )
				strExistingIDs.TrimRight(",");
				strExistingIDs += ")";
				if (strExistingIDs != "()") {
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM PatientWellnessCompletionItemT WHERE ID IN %s", strExistingIDs);
				}

			}

			//loop through the completion items and see if they are all finished
			CheckCompleted();
			if (m_bComplete) {
				
				COleDateTime dtNull;
				dtNull.SetDate(1800,12,31);
				if (m_dtComplete == dtNull) {
					//they completed them all right now
					AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientWellnessT SET CompletedDate = getDate() WHERE ID = %li", m_nWellnessID);
				}
				//don't do anything here because it means it was finished already
			}		
			else {
				//unset the date
				AddStatementToSqlBatch(strSqlBatch, "UPDATE PatientWellnessT SET CompletedDate = NULL WHERE ID = %li", m_nWellnessID);
			}

			//let's start with WellnessTemplateT
			AddStatementToSqlBatch(strSqlBatch, 
				" UPDATE PatientWellnessT SET Guideline = '%s', Reference = '%s', "
				" Note = '%s', FirstPresentedDate = '%s', ModifiedDate = GetDate(), "
				" ModifiedUserID = %li WHERE ID = %li",
				_Q(strGuideline), _Q(strReference), _Q(strNote),
				FormatDateTimeForSql(dtFirstPresented, dtoDate), GetCurrentUserID(), m_nWellnessID);


			//now loop to save what is still there
			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_pCompletionList->GetFirstRow();
			while (pRow) {

				long nCompletionID = VarLong(pRow->GetValue(cicID), -1);

				long nRecordID = VarLong(pRow->GetValue(cicRecordID));
				BYTE nRecordType = VarByte(pRow->GetValue(cicRecordType));
				CString strCompletionDate;
				CString strCompletionValue;
				long nIsCompleted = VarLong(pRow->GetValue(cicStatus));
				BOOL bIsAutoComplete = FALSE;
				CString strNote;
				if (nIsCompleted == -1) {
					strCompletionDate = "NULL";						
					strCompletionValue = "NULL";						
				}
				else if (nIsCompleted == -2) {

					//it's completed, either manually or auto
					_variant_t varCompletionRecord = pRow->GetValue(cicCompletionRecordID);
					
					if (varCompletionRecord.vt == VT_NULL) {
						//its manually completed
						strCompletionValue = "''";
						strCompletionDate = "'" + FormatDateTimeForSql(VarDateTime(pRow->GetValue(cicDate))) + "'";
					}
					else {
						bIsAutoComplete = TRUE;
					}
				}
				else if (nIsCompleted == -3) {
					
					//skipped								
					strCompletionDate = "'" + FormatDateTimeForSql(VarDateTime(pRow->GetValue(cicDate))) + "'";
					strCompletionValue = "NULL";
					
				}				

				strNote = VarString(pRow->GetValue(cicNote));
				if (nCompletionID == -1) {				

					//its a new row and cannot be auto-completed yet
					//TES 6/8/2009 - PLID 34505 - Added RecordType
					AddStatementToSqlBatch(strSqlBatch, 
						" INSERT INTO PatientWellnessCompletionItemT "
						" (RecordID, RecordType, CompletionValue, CompletionDate, Note, PatientWellnessID) "
						" VALUES (%li, %i, %s, %s, '%s', %li) ",
						nRecordID, nRecordType, 
						strCompletionValue, strCompletionDate, _Q(strNote), m_nWellnessID);

				}
				else {
					
					//its an existing row
					if (bIsAutoComplete) {
						
						//we don't allow them to change anything
						/*AddStatementToSqlBatch(strSqlBatch, 
							" UPDATE PatientWellnessCompletionItemT SET "
							" CompletionDate = %s, Note = '%s' "
							" WHERE ID = %li",
							strCompletionDate, _Q(strNote), nCompletionID);*/
					}
					else {

						//TES 6/8/2009 - PLID 34505 - Added RecordType
						AddStatementToSqlBatch(strSqlBatch, 
							" UPDATE PatientWellnessCompletionItemT SET "
							" CompletionDate = %s, CompletionRecordID = NULL, CompletionValue = %s, Note = '%s' "
							" WHERE ID = %li",
							strCompletionDate, strCompletionValue, _Q(strNote), nCompletionID);
					}				
				}
				
				//add to our EMRInfoItems
				m_aryCompletionRecordIDs.Add(nRecordID);
				m_aryCompletionRecordTypes.Add(nRecordType);
				pRow = pRow->GetNextRow();
			}

			//they aren't allowed to change the criteria on a patient template, so we are done
			ExecuteSqlBatch(strSqlBatch);

			//call the completeion function
			//TES 6/8/2009 - PLID 34505 - Added an array of RecordTypes
			UpdatePatientWellnessQualification_CompletionItemsCompleted(GetRemoteData(), m_nPatientID, m_aryCompletionRecordIDs, m_aryCompletionRecordTypes);


			return TRUE;
		}

	}NxCatchAll("Error in CPatientWellnessAlertDlg::Save()");

	return FALSE;
}

void CPatientWellnessAlertDlg::EditingFinishedPatWellAlertCriteria(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		if (bCommit) {
		
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if (pRow) {

				switch (nCol) {

					case clcOperator:
					{						
						WellnessTemplateCriterionType wtct = (WellnessTemplateCriterionType)VarByte(pRow->GetValue(clcTypeID));
						WellnessTemplateCriteriaOperator wtco = (WellnessTemplateCriteriaOperator)VarByte(varNewValue);
						EmrInfoType eit = (EmrInfoType)VarShort(pRow->GetValue(clcEMRInfoType),-1);
						//TTrack whether we escape this value for a LIKE clause.
						bool bEscaped = false;
						if(wtco == wtcoFilledIn || wtco == wtcoNotFilledIn || wtco == wtcoExists || wtco == wtcoDoesNotExist) {
							//TES 5/22/2009 - PLID 34302 - The value is not used with this operator, so clear it out.
							CString strCurrentValue = VarString(pRow->GetValue(clcValue),"");
							if(!strCurrentValue.IsEmpty()) {																
								pRow->PutValue(clcValue, _bstr_t(""));
							}
							pRow->PutValue(clcDisplayValue, _bstr_t(""));
						}
						else {
							//TES 6/8/2009 - PLID 34510 - Make sure this is actually an EMR-Item-based criterion
							if(wtct == wtctEmrItem && eit == eitMultiList) {
								CString strCurrentValue = VarString(pRow->GetValue(clcValue),"");
								
								if(strCurrentValue.IsEmpty()) {
									//TES 6/2/2009 - PLID 34302 - Restore the Select option
									pRow->PutValue(clcDisplayValue, _bstr_t("<Select...>"));
								}
							}
							if(wtco != wtcoEqual && wtco != wtcoNotEqual) {
								//TES 5/22/2009 - PLID 34302 - With any operator other than equal or not equal, they
								// can only have one selected item in the Value field, so truncate the list to one entry.
								CString strCurrentValue = VarString(pRow->GetValue(clcValue),"");
								strCurrentValue.TrimLeft(" ");
								int nSpace = strCurrentValue.Find(" ");
								if(nSpace != -1) {
									CString strNewValue = strCurrentValue.Left(nSpace);
									pRow->PutValue(clcValue, _bstr_t(strNewValue));
									pRow->PutValue(clcDisplayValue, GetTableField("EmrDataT INNER JOIN EmrInfoMasterT ON EmrDataT.EmrInfoID = EmrInfoMasterT.ActiveEmrInfoID",
										"Data", "EmrDataGroupID", atol(strNewValue)));
								}
							}
							else if(wtco == wtcoContains || wtco == wtcoDoesNotContain) {
								if(wtct == wtctEmrProblemList || (wtct == wtctEmrItem && eit == eitText)) {
									//With this operator/type combination, we want to
									// store the text escaped for a LIKE clause
									bEscaped = true;
									CString strValue = VarString(pRow->GetValue(clcDisplayValue),"");
									CString strNewValue = FormatForLikeClause(strValue);
									pRow->PutValue(clcValue, _bstr_t(strNewValue));									
								}
							}
						}
						
						WellnessTemplateCriteriaOperator wtcoOld = (WellnessTemplateCriteriaOperator)VarByte(varOldValue);
						if( (wtct == wtctEmrProblemList || (wtct == wtctEmrItem && eit == eitText)) && 
							(wtcoOld == wtcoContains || wtcoOld == wtcoDoesNotContain) &&
							!bEscaped) {
								//This used to be escaped, but isn't any more.  Update the
								// database and the datalist.
								CString strOldValue = VarString(pRow->GetValue(clcValue),"");
								CString strNewValue = UnformatFromLikeClause(strOldValue);								
								pRow->PutValue(clcValue, _bstr_t(strNewValue));
						}
						if(wtct == wtctEmrItem && eit == eitMultiList) {
							//Need to update the format, hyperlink-ness may have changed
							pRow->PutRefCellFormatOverride(clcDisplayValue, GetValueFormatSettings(wtct, eit, VarLong(pRow->GetValue(clcRecordID),-1), wtco));
						}
					}
					//For some reason the datalist wasn't redrawing itself here, so make
					// sure that it does.
					m_pCriteriaList->SetRedraw(TRUE);
					break;

					case clcDisplayValue:
						{
							//copy it to the value field, as a string
							//TES 6/12/2009 - PLID 34567 - If they are using the Age (Years) option, then we want
							// to copy the value in in months (i.e., the entered value * 12) to the "real" value field.
							WellnessTemplateCriterionType wtct = (WellnessTemplateCriterionType)VarByte(pRow->GetValue(clcTypeID));
							_variant_t varValue = varNewValue;
							if (wtct == wtctAge && VarString(pRow->GetValue(clcName)) == "Age (Years)") {
								CString strValue = AsString(varNewValue);
								if(!strValue.IsEmpty()) {
									long nNewValue = atol(strValue);
									if(nNewValue > 0) {
										varValue = nNewValue*12;
									}
								}
							}
							pRow->PutValue(clcValue, _variant_t(AsString(varValue)));
						}
					break;

					case clcLastXDays:
						//make sure its positive and a number
						/*CString strUserEntered = VarString(varNewValue, "");
						long nValue = atol(strUserEntered);
						if(nValue <= 0) {
							pRow->PutValue(clcLastXDays, g_cvarNull);
						}
						else {
							pRow->PutValue(clcLastXDays, _variant_t(nValue));
						}*/
					break;

				}			
			}

			m_bHasChanged = TRUE;
			AddToChangedCriteriaList(lpRow);
			
		}

	}NxCatchAll("Error in CPatientWellnessAlertDlg::EditingFinishedPatWellAlertCriteria");
}

void CPatientWellnessAlertDlg::RequeryFinishedPatWellAlertCompleteItems(short nFlags)
{
	try {
		if (!m_bIsTemplate) {
			CheckCompleted();
		}
	}NxCatchAll("Error in CPatientWellnessAlertDlg::RequeryFinishedPatWellAlertCompleteItems");
}

// (j.gruber 2009-06-01 09:48) - PLID 34401 - reports
void CPatientWellnessAlertDlg::OnBnClickedPatWellnessPreviewGuide()
{
	try {

		//let's save this first
		if (Save()) {

			//print this on the fly
			CString str = (LPCTSTR)m_reGuidelines->RichText;

			//need to take out the nxheader crap
			CString strRichText = ConvertTextFormat(str, tfNxRichText, tfRichText);

			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(665)]);
		
			infReport.strExtraText = strRichText;
			infReport.strExtraField = m_strTemplateName;
			infReport.strExtendedSql = GetExistingPatientName(m_nPatientID);
			//(e.lally 2010-09-10) PLID 40488 - Send in the PatientID too
			infReport.nExtraID = m_nPatientID;
		
			RunReport(&infReport, true, this, "Patient Wellness Guidelines");

			//now close the dialog
			EndDialog(ID_PREVIEW_REPORT);
		}

	}NxCatchAll("Error in CPatientWellnessAlertDlg::OnBnClickedPatWellnessPreviewGuide()");
}
// (j.gruber 2009-06-01 09:48) - PLID 34401 - reports
void CPatientWellnessAlertDlg::OnBnClickedPatWellnessPreviewRef()
{
	try {

		//let's save this first
		if (Save()) {

			//print this on the fly
			CString str = (LPCTSTR)m_reReferenceMaterials->RichText;

			//need to take out the nxheader crap
			CString strRichText = ConvertTextFormat(str, tfNxRichText, tfRichText);

			CReportInfo infReport(CReports::gcs_aryKnownReports[CReportInfo::GetInfoIndex(666)]);
		
			infReport.strExtraText = strRichText;
			infReport.strExtraField = m_strTemplateName;
			infReport.strExtendedSql = GetExistingPatientName(m_nPatientID);
			//(e.lally 2010-09-10) PLID 40488 - Send in the PatientID too
			infReport.nExtraID = m_nPatientID;
		
			RunReport(&infReport, true, this, "Patient Wellness References");

			//now close the dialog			
			EndDialog(ID_PREVIEW_REPORT);
		}

	}NxCatchAll("Error in CPatientWellnessAlertDlg::OnBnClickedPatWellnessPreviewRef()");
}

void CPatientWellnessAlertDlg::EditingFinishingPatWellAlertCompleteItems(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		if (nCol == cicDate) {

			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if (pRow) {

				if(pvarNewValue->vt != VT_DATE || VarDateTime(*pvarNewValue).GetStatus() != COleDateTime::valid || VarDateTime(*pvarNewValue).GetYear() <= 1899) {
					//don't save					
					*pbCommit = FALSE;				
				}
			}
		}
	}NxCatchAll("Error in CPatientWellnessAlertDlg::EditingFinishingPatWellAlertCompleteItems");
}

// (j.gruber 2009-06-03 12:55) - PLID 34457 - use close button instead of ok/cancel
void CPatientWellnessAlertDlg::OnBnClickedClose()
{
	try { 
		EndDialog(IDCANCEL);
	}NxCatchAll("Error in CPatientWellnessAlertDlg::OnBnClickedClose()");
}

void CPatientWellnessAlertDlg::EditingFinishingPatWellAlertCriteria(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}
		switch(nCol) {
			case clcLastXDays:
				{
					//TES 5/22/2009 - PLID 34302 - Force a positive integer.
					long nValue = atol(strUserEntered);
					if(nValue <= 0) {
						*pvarNewValue = g_cvarNull;
					}
					else if(nValue > 36500) {
						MsgBox("You cannot enter a value in the 'Last X Days' column greater than 100 years");
						*pbCommit = FALSE;
					}
					else {
						*pvarNewValue = _variant_t(nValue);
					}
				}
			break;	
			case clcDisplayValue:
				{
					WellnessTemplateCriterionType wtct = (WellnessTemplateCriterionType)VarByte(pRow->GetValue(clcTypeID));
					switch(wtct) {
						case wtctAge:
							{
								//Force a positive integer
								long nValue = atol(strUserEntered);
								if(nValue <= 0) {
									*pvarNewValue = g_cvarNull;
								}
								// (b.cardillo 2009-06-11 18:01) - PLID 34609 - Eventually SQL will prevent super-extreme 
								// values, but we also protect from plain nonsense values.
								else if(nValue > 4800) {
									MsgBox("You cannot enter a value in the 'Value' column greater than 400 years");
									*pbCommit = FALSE;
								}
								else {
									*pvarNewValue = _variant_t(nValue);
								}
							}
							break;
						case wtctEmrItem:
							{
								EmrInfoType eit = (EmrInfoType)VarByte(pRow->GetValue(clcEMRInfoType),-1);
								switch(eit) {
									case eitSlider:
										{
											//Force a valid number
											double dValue = atof(strUserEntered);
											*pvarNewValue = _variant_t(dValue);
										}
										break;
								}
							}
							break;
					}
				}
			break;
		}			
	
	}NxCatchAll("Error in CPatientWellnessAlertDlg::EditingFinishingPatWellAlertCriteria");
}

BOOL CPatientWellnessAlertDlg::ValidateCurrentTemplate()
{
	try {	

		//Are there any criteria?
		if(m_pCriteriaList->GetRowCount() == 0) {
			MsgBox("Please add one or more criteria.");
			return FALSE;
		}

		//Validate that each criteria has a value, unless the operator is Filled In/Not Filled In.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCriteriaList->GetFirstRow();
		while(pRow) {
			CString strValue = AsString(pRow->GetValue(clcValue));
			WellnessTemplateCriteriaOperator wtco = (WellnessTemplateCriteriaOperator)VarByte(pRow->GetValue(clcOperator));
			if(strValue.IsEmpty() && wtco != wtcoFilledIn && wtco != wtcoNotFilledIn && wtco != wtcoExists && wtco != wtcoDoesNotExist) {
				MsgBox("The criterion for %s does not have a value entered.  Please enter a value for this criterion to be compared to.", 
					VarString(pRow->GetValue(clcName)));
				return FALSE;
			}
			pRow = pRow->GetNextRow();
		}

		return TRUE;
	}NxCatchAll("Error in CPatientWellnessAlertDlg::ValidateCurrentTemplate()");
	return FALSE;
}
