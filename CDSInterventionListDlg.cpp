// CDSInterventionListDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "CDSInterventionListDlg.h"
#include "CDSInterventionDlg.h"


// CCDSInterventionListDlg dialog
//TES 11/1/2013 - PLID 59276 - Created

IMPLEMENT_DYNAMIC(CCDSInterventionListDlg, CNxDialog)

CCDSInterventionListDlg::CCDSInterventionListDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCDSInterventionListDlg::IDD, pParent)
{
	m_nPatientID = -1;
}

CCDSInterventionListDlg::~CCDSInterventionListDlg()
{
}

void CCDSInterventionListDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCDSInterventionListDlg)
	DDX_Control(pDX, IDOK, m_nxbClose);
	DDX_Control(pDX, IDC_CDS_LIST_NXC, m_bkg);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCDSInterventionListDlg, CNxDialog)
	ON_BN_CLICKED(IDC_INCLUDE_ACKNOWLEDGED, &CCDSInterventionListDlg::OnBnClickedIncludeAcknowledged)
END_MESSAGE_MAP()

using namespace NXDATALIST2Lib;

enum CdsInterventionListColumns
{
	cilcID = 0,
	cilcPatName = 1,
	cilcRuleName = 2,
	cilcCreatedDate = 3,
	cilcAckBy = 4,
	cilcAckDate = 5,
	cilcView = 6,
};

// CCDSInterventionListDlg message handlers
BOOL CCDSInterventionListDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {
		m_nxbClose.AutoSet(NXB_CLOSE);
		m_pInterventionList = BindNxDataList2Ctrl(IDC_CDS_INTERVENTION_LIST, false);

		//We've been given either a patient ID, or a list of specific IDs. Load accordingly.
		if(m_nPatientID != -1) {
			SetDlgItemText(IDC_CDS_INTERVENTION_LABEL, m_strPatientName + " has triggered the following Clinical Decision Support rules, "
				"which require your attention. Please review each of the interventions in this list and ensure the appropriate "
				"followup action had been taken.");
			CString strWhere;
			strWhere.Format("DecisionRuleInterventionsT.PatientID = %li AND DecisionRuleInterventionsT.AcknowledgedBy Is Null",
				m_nPatientID);
			m_pInterventionList->WhereClause = _bstr_t(strWhere);
			m_pInterventionList->Requery();

			m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1, m_nPatientID));

			//Hide the patient name column, it's superfluous here
			m_pInterventionList->GetColumn(cilcPatName)->PutColumnStyle(csVisible|csFixedWidth);
			m_pInterventionList->GetColumn(cilcPatName)->PutStoredWidth(0);

			//Hide the acknowledged columns, since for the moment we're filtering out acknowledged interventions
			m_pInterventionList->GetColumn(cilcAckBy)->PutColumnStyle(csVisible|csFixedWidth);
			m_pInterventionList->GetColumn(cilcAckBy)->PutStoredWidth(0);
			m_pInterventionList->GetColumn(cilcAckDate)->PutColumnStyle(csVisible|csFixedWidth);
			m_pInterventionList->GetColumn(cilcAckDate)->PutStoredWidth(0);

		}
		else {
			ASSERT(m_arInterventionIDs.GetCount() > 0);
			SetDlgItemText(IDC_CDS_INTERVENTION_LABEL, "The following Clinical Decision Support rules have been triggered for "
				"one or more of your patients, and require your attention. Please review each of the interventions in this list "
				"and ensure the appropriate followup action has been taken.");
			CString strWhere;
			strWhere.Format("DecisionRuleInterventionsT.ID IN (%s) AND DecisionRuleInterventionsT.AcknowledgedBy Is Null", ArrayAsString(m_arInterventionIDs));
			m_pInterventionList->WhereClause = _bstr_t(strWhere);
			m_pInterventionList->Requery();

			m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

			//Hide the acknowledged columns, they're all empty
			m_pInterventionList->GetColumn(cilcAckBy)->PutColumnStyle(csVisible|csFixedWidth);
			m_pInterventionList->GetColumn(cilcAckBy)->PutStoredWidth(0);
			m_pInterventionList->GetColumn(cilcAckDate)->PutColumnStyle(csVisible|csFixedWidth);
			m_pInterventionList->GetColumn(cilcAckDate)->PutStoredWidth(0);

			//Hide the checkbox to "Include Acknowledged"
			GetDlgItem(IDC_INCLUDE_ACKNOWLEDGED)->ShowWindow(SW_HIDE);
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCDSInterventionListDlg::OpenWithNewInterventions(const CDWordArray &arInterventions)
{
	//TES 11/10/2013 - PLID 59400 - If they can't read interventions, don't show the dialog
	if(!(GetCurrentUserPermissions(bioCDSInterventions) & (sptRead|sptReadWithPass)) ) {
		return;
	}
	//Clear the Patient ID, set our array
	m_nPatientID = -1;
	m_arInterventionIDs.RemoveAll();
	m_arInterventionIDs.Append(arInterventions);
	DoModal();
}

void CCDSInterventionListDlg::OpenWithPatientInterventions(long nPatientID, const CString &strPatientName)
{
	//TES 11/10/2013 - PLID 59400 - If they can't read interventions, don't show the dialog
	if(!(GetCurrentUserPermissions(bioCDSInterventions) & (sptRead|sptReadWithPass)) ) {
		return;
	}
	//Set our patient information, clear our array
	m_nPatientID = nPatientID;
	m_strPatientName = strPatientName;
	m_arInterventionIDs.RemoveAll();
	DoModal();
}

BEGIN_EVENTSINK_MAP(CCDSInterventionListDlg, CNxDialog)
ON_EVENT(CCDSInterventionListDlg, IDC_CDS_INTERVENTION_LIST, 19, CCDSInterventionListDlg::LeftClickCdsInterventionList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

void CCDSInterventionListDlg::LeftClickCdsInterventionList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//If they clicked on the View... hyperlink, open up the individual alert
		if(nCol == cilcView) {
			//TES 11/10/2013 - PLID 59400 - Check their permission to read interventions first
			if(!CheckCurrentUserPermissions(bioCDSInterventions, sptRead)) {
				return;
			}
			CCDSInterventionDlg dlg;
			dlg.m_nInterventionID = VarLong(pRow->GetValue(cilcID));
			dlg.DoModal();
			m_pInterventionList->Requery();
		}
	}NxCatchAll(__FUNCTION__);
}

void CCDSInterventionListDlg::OnBnClickedIncludeAcknowledged()
{
	try {
		//This is only valid if we're filtering by patient
		if(m_nPatientID != -1) {
			//Requery the list
			BOOL bIncludeAcknowledged = IsDlgButtonChecked(IDC_INCLUDE_ACKNOWLEDGED);
			CString strAck = bIncludeAcknowledged ? "" : " AND DecisionRuleInterventionsT.AcknowledgedBy Is Null";
			CString strWhere;
			strWhere.Format("DecisionRuleInterventionsT.PatientID = %li %s",
				m_nPatientID, strAck);
			m_pInterventionList->WhereClause = _bstr_t(strWhere);
			m_pInterventionList->Requery();

			if(bIncludeAcknowledged) {
				m_pInterventionList->GetColumn(cilcAckBy)->PutColumnStyle(csVisible|csWidthData);
				m_pInterventionList->GetColumn(cilcAckDate)->PutColumnStyle(csVisible|csWidthData);
			}
			else {
				m_pInterventionList->GetColumn(cilcAckBy)->PutColumnStyle(csVisible|csFixedWidth);
				m_pInterventionList->GetColumn(cilcAckBy)->PutStoredWidth(0);
				m_pInterventionList->GetColumn(cilcAckDate)->PutColumnStyle(csVisible|csFixedWidth);
				m_pInterventionList->GetColumn(cilcAckDate)->PutStoredWidth(0);
			}

		}
	}NxCatchAll(__FUNCTION__);
}
