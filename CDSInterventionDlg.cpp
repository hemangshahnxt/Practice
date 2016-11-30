// CDSInterventionDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "CDSInterventionDlg.h"
#include "DecisionSupportManager.h"
#include <InternationalUtils.h>

// CCDSInterventionDlg dialog
//TES 11/1/2013 - PLID 59276 - Created

IMPLEMENT_DYNAMIC(CCDSInterventionDlg, CNxDialog)

CCDSInterventionDlg::CCDSInterventionDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCDSInterventionDlg::IDD, pParent)
{
	m_bSavedAck = m_bAcknowledged = FALSE;
	m_nSavedAckBy = m_nAckBy = -1;
}

CCDSInterventionDlg::~CCDSInterventionDlg()
{
}

void CCDSInterventionDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CDS_INTERVENTION_NXC_1, m_bkg1);
	DDX_Control(pDX, IDC_CDS_INTERVENTION_NXC_2, m_bkg2);
	DDX_Control(pDX, IDC_CDS_INTERVENTION_NXC_3, m_bkg3);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	DDX_Control(pDX, IDC_GO_TO_CITATION, m_nxbGoToCitation);
}


BEGIN_MESSAGE_MAP(CCDSInterventionDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ACKNOWLEDGE_CDS_INTERVENTION, &CCDSInterventionDlg::OnAcknowledgeCdsIntervention)
	ON_BN_CLICKED(IDC_GO_TO_CITATION, &CCDSInterventionDlg::OnBnClickedGoToCitation)
END_MESSAGE_MAP()


using namespace ADODB;
using namespace Intervention;

// CCDSInterventionDlg message handlers
BOOL CCDSInterventionDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		//Load the information
		_RecordsetPtr rsIntervention = CreateParamRecordset("SELECT DecisionRuleID, DecisionRulesT.Name, PersonT.FullName, DecisionRuleInterventionsT.DateCreated, "
			"DecisionRuleInterventionsT.PatientID, DecisionRuleInterventionsT.AcknowledgedBy, DecisionRuleInterventionsT.AcknowledgedDate, "
			"DecisionRuleInterventionsT.AcknowledgeNote, UsersT.UserName AS AcknowledgedByUserName "
			"FROM DecisionRuleInterventionsT INNER JOIN DecisionRulesT ON DecisionRuleInterventionsT.DecisionRuleID = DecisionRulesT.ID "
			"INNER JOIN PersonT ON DecisionRuleInterventionsT.PatientID = PersonT.ID "
			"LEFT JOIN UsersT ON DecisionRuleInterventionsT.AcknowledgedBy = UsersT.PersonID "
			"WHERE DecisionRuleInterventionsT.ID = {INT}", m_nInterventionID);
		//We should have been given a valid ID
		if(rsIntervention->eof) {
			ThrowNxException("Invalid CDS InterventionID %li", m_nInterventionID);
		}
		else {
			//Leverage the existing class to load the information stemming from the original template.
			m_pConfigurationManager.reset(new DecisionSupportManager());
			IInterventionTemplatePtr pTemplate;
			pTemplate.reset();

			long nTemplateID = AdoFldLong(rsIntervention, "DecisionRuleID");
			CString strTemplateName = AdoFldString(rsIntervention, "Name");
			pTemplate = m_pConfigurationManager->GetTemplatePtr(strTemplateName, nTemplateID);
			m_pConfigurationManager->SetSelectedTemplate(pTemplate);
			
			SetDlgItemText(IDC_DECISION_RULE_NAME, pTemplate->GetName());
			SetDlgItemText(IDC_CDS_REFERENCE_INFORMATION, pTemplate->GetMaterials());
			SetDlgItemText(IDC_CITATION, pTemplate->GetGuidelines());
			//TES 12/5/2013 - PLID 59276 - If the Citation field is empty, disable the "Go" button
			if(pTemplate->GetGuidelines().IsEmpty()) {
				m_nxbGoToCitation.EnableWindow(FALSE);
			}
			SetDlgItemText(IDC_DEVELOPER, pTemplate->GetDeveloper());
			SetDlgItemText(IDC_FUNDING_SOURCE, pTemplate->GetFundingInfo());
			COleDateTime dt = pTemplate->GetReleaseDate();
			if(dt.GetYear() != 1899) {
				SetDlgItemText(IDC_RELEASE_DATE, FormatDateTimeForInterface(dt, dtoDate));
			}
			dt = pTemplate->GetRevisionDate();
			if(dt.GetYear() != 1899) {
				SetDlgItemText(IDC_REVISION_DATE, FormatDateTimeForInterface(dt, dtoDate));
			}

			m_pCriteriaList = BindNxDataList2Ctrl(IDC_CDS_INTERVENTION_CRITERIA_LIST, false);
			m_pCriteriaList->FromClause = _bstr_t(m_pConfigurationManager->GetTemplateCriteriaFromClause());
			m_pCriteriaList->WhereClause = _bstr_t(m_pConfigurationManager->GetTemplateCriteriaWhereClause());
			m_pCriteriaList->Requery();

			//Now load the information from this specific intervention
			SetWindowText("CDS Intervention for " + AdoFldString(rsIntervention, "FullName"));
			m_nxtInputDate = BindNxTimeCtrl(this, IDC_INTERVENTION_INPUT_DATE);
			m_nxtInputDate->SetDateTime(AdoFldDateTime(rsIntervention, "DateCreated"));

			COLORREF clr = GetNxColor(GNC_PATIENT_STATUS, 1, AdoFldLong(rsIntervention, "PatientID"));
			m_bkg1.SetColor(clr);
			m_bkg2.SetColor(clr);
			m_bkg3.SetColor(clr);

			m_nSavedAckBy = AdoFldLong(rsIntervention, "AcknowledgedBy", -1);
			if(m_nSavedAckBy != -1) {
				m_bSavedAck = TRUE;
				m_dtSavedAckDate = AdoFldDateTime(rsIntervention, "AcknowledgedDate");
				m_strSavedAckByName = AdoFldString(rsIntervention, "AcknowledgedByUserName", "");
				CheckDlgButton(IDC_ACKNOWLEDGE_CDS_INTERVENTION, BST_CHECKED);
				OnAcknowledgeCdsIntervention();
			}
			else {
				m_bSavedAck = FALSE;
			}

			m_strSavedAckNote = AdoFldString(rsIntervention, "AcknowledgeNote", "");
			SetDlgItemText(IDC_ACKNOWLEDGMENT_NOTES_EDIT, m_strSavedAckNote);

			//TES 11/10/2013 - PLID 59400 - Disable the editable controls if they don't have write permission
			if(!(GetCurrentUserPermissions(bioCDSInterventions) & (sptWrite|sptWriteWithPass)) ) {
				GetDlgItem(IDC_ACKNOWLEDGE_CDS_INTERVENTION)->EnableWindow(FALSE);
				GetDlgItem(IDC_ACKNOWLEDGMENT_NOTES_EDIT)->EnableWindow(FALSE);
			}
			
		}

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
void CCDSInterventionDlg::OnAcknowledgeCdsIntervention()
{
	try {
		if(IsDlgButtonChecked(IDC_ACKNOWLEDGE_CDS_INTERVENTION)) {
			//OK, we're acknowledged, so fill in the label. If we started out acknowledged, then don't change the user and time,
			// but leave them at their original values
			m_bAcknowledged = TRUE;
			CString strAckInfo;
			if(m_bSavedAck) {
				strAckInfo.Format("Acknowledged by %s on %s", m_strSavedAckByName, FormatDateTimeForInterface(m_dtSavedAckDate));
			}
			else {
				m_nAckBy = GetCurrentUserID();
				m_dtAckDate = COleDateTime::GetCurrentTime();
				strAckInfo.Format("Acknowledged by %s on %s", GetCurrentUserName(), FormatDateTimeForInterface(m_dtAckDate));
			}
			SetDlgItemText(IDC_ACKNOWLEDGMENT_INFO, strAckInfo);
		}
		else {
			//Not acknowledged, clear out IDC_ACKNOWLEDGMENT_INFO
			m_bAcknowledged = FALSE;
			SetDlgItemText(IDC_ACKNOWLEDGMENT_INFO, "");
		}
			
	}NxCatchAll(__FUNCTION__);
}

void CCDSInterventionDlg::OnOK()
{
	try {
		//TES 11/10/2013 - PLID 59400 - Make sure we don't check permissions more than once
		bool bPermsChecked = false;

		//If the acknowledgement checkbox has changed, save that information.
		if(m_bAcknowledged != m_bSavedAck) {
			//TES 11/10/2013 - PLID 59400 - Check their write permission
			if(!CheckCurrentUserPermissions(bioCDSInterventions, sptWrite)) {
				return;
			}
			bPermsChecked = true;

			if(m_bAcknowledged) {
				if(m_nAckBy == -1 || m_dtAckDate.GetStatus() != COleDateTime::valid) {
					AfxThrowNxException("Inconsistent acknowledgment information in CCDSInterventionDlg!");
				}
				ExecuteParamSql("UPDATE DecisionRuleInterventionsT SET AcknowledgedBy = {INT}, AcknowledgedDate = {OLEDATETIME} "
					"WHERE ID = {INT}", m_nAckBy, m_dtAckDate, m_nInterventionID);
			}
			else {
				ExecuteParamSql("UPDATE DecisionRuleInterventionsT SET AcknowledgedBy = NULL, AcknowledgedDate = NULL "
					"WHERE ID = {INT}", m_nInterventionID);
			}
		}

		//If the acknowledgement notes have changed, save them.
		CString strAckNote;
		GetDlgItemText(IDC_ACKNOWLEDGMENT_NOTES_EDIT, strAckNote);
		if(strAckNote != m_strSavedAckNote) {
			//TES 11/10/2013 - PLID 59400 - Check their write permission (if we haven't already)
			if(!bPermsChecked) {
				if(!CheckCurrentUserPermissions(bioCDSInterventions, sptWrite)) {
					return;
				}
			}
			ExecuteParamSql("UPDATE DecisionRuleInterventionsT SET AcknowledgeNote = {STRING} "
				"WHERE ID = {INT}", strAckNote, m_nInterventionID);
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CCDSInterventionDlg, CNxDialog)
ON_EVENT(CCDSInterventionDlg, IDC_CDS_INTERVENTION_CRITERIA_LIST, 18, CCDSInterventionDlg::RequeryFinishedCdsInterventionCriteriaList, VTS_I2)
END_EVENTSINK_MAP()

void CCDSInterventionDlg::RequeryFinishedCdsInterventionCriteriaList(short nFlags)
{
	try {
		//TES 11/27/2013 - PLID 59276 - We need to set all the cell formats to be the same as they are on the template, so
		// that we don't have weird issues such as Filter IDs showing instead of names
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCriteriaList->GetFirstRow();
		while(pRow) {

			IInterventionCriterionPtr pCriterion = m_pConfigurationManager->LoadCriterionFromRow(pRow);
			pCriterion->LoadIntoRow(pRow);
				
			pRow = pRow->GetNextRow();
		}
	}NxCatchAll(__FUNCTION__);
}

void CCDSInterventionDlg::OnBnClickedGoToCitation()
{
	try {
		//TES 12/5/2013 - PLID 59276 - Attempt to open the citation (could be a URL, or a link to a .pdf, or whatever).
		CString strCitation;
		GetDlgItemText(IDC_CITATION, strCitation);
		ShellExecute(NULL, NULL, strCitation, NULL, NULL, SW_SHOW);
	}NxCatchAll(__FUNCTION__);
}
