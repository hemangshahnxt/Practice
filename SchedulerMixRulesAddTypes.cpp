// SchedulerMixRulesAddTypes.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "SchedulerMixRulesAddTypes.h"


// CSchedulerMixRulesAddTypes dialog

IMPLEMENT_DYNAMIC(CSchedulerMixRulesAddTypes, CNxDialog)
enum appttypeList{
	appttypeID = 0,
	appttypeName,
};

enum apptPurposeList{
	apptPurposeID,
	apptPurposeName,
};
// (s.tullis 2014-12-30 14:39) - PLID 64144 - Constructor
CSchedulerMixRulesAddTypes::CSchedulerMixRulesAddTypes(long nRuleID, long nResourceID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CSchedulerMixRulesAddTypes::IDD, pParent)
{
	m_RuleID = nRuleID;
	m_ResourceID = nResourceID;
}

CSchedulerMixRulesAddTypes::~CSchedulerMixRulesAddTypes()
{
}

void CSchedulerMixRulesAddTypes::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MAX_APPT, m_editMaxAppt);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnSave);
	DDX_Control(pDX, IDC_RULE_DETAIL_SAVE_ADD_ANOTHER, m_btnSaveAndAddAnother);
}


BEGIN_MESSAGE_MAP(CSchedulerMixRulesAddTypes, CNxDialog)
	ON_EN_CHANGE(IDC_MAX_APPT, &CSchedulerMixRulesAddTypes::OnEnChangeMaxAppt)
	ON_BN_CLICKED(IDC_RULE_DETAIL_SAVE_ADD_ANOTHER, &CSchedulerMixRulesAddTypes::OnBnClickedRuleDetailSaveAddAnother)
	ON_BN_CLICKED(IDOK, &CSchedulerMixRulesAddTypes::OnBnClickedOk)
END_MESSAGE_MAP()


// CSchedulerMixRulesAddTypes message handlers


BOOL CSchedulerMixRulesAddTypes::OnInitDialog(){

	CNxDialog::OnInitDialog();

	try{
		m_btnSave.AutoSet(NXB_OK);
		m_btnSaveAndAddAnother.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pApptType = BindNxDataList2Ctrl(IDC_RULE_APPT_TYPE, true);
		m_pApptPurpose = BindNxDataList2Ctrl(IDC_RULE_APPT_PURPOSE, true);

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptType->GetNewRow();
		if (pRow){
			pRow->PutValue(appttypeList::appttypeID, -1);
			pRow->PutValue(appttypeList::appttypeName, "< All Appointments Types >");
			m_pApptType->AddRowSorted(pRow, NULL);
		}

	
		m_pApptPurpose->PutEnabled(VARIANT_FALSE);
		m_editMaxAppt.EnableWindow(FALSE);

	}NxCatchAll(__FUNCTION__)

		return TRUE;
}
// (s.tullis 2014-12-30 14:39) - Make sure the Max Appt is not negative
void CSchedulerMixRulesAddTypes::OnEnChangeMaxAppt()
{
	CString strWindowText;
	m_editMaxAppt.GetWindowText(strWindowText);
	m_nMaxNumAppt = atol(strWindowText);
	// need to only allow decimal numbers no negatives... no 3.56 and such 
	if (m_nMaxNumAppt < 0){
		MessageBox("Max # of Appointments must be greater than or equal to 0 ");
		m_editMaxAppt.SetWindowText("");
		return;
	}
}

// (s.tullis 2014-12-30 14:39) - Save -- Pass back to config dialog
BOOL CSchedulerMixRulesAddTypes::Save()
{
	try{
	
		long nApptTypeID=-1;
		long nApptPurposeID =-1;
		CString strEditWindowText="";
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pApptType->GetCurSel();
	
		
		
	
		if (pRow){
			nApptTypeID=VarLong(pRow->GetValue(appttypeList::appttypeID), -1);
			
			}
		else{
			MessageBox("Please select an appointment type in order to save. ");
			return FALSE;
		}

		if (m_pApptPurpose->Enabled == VARIANT_TRUE){
			pRow = m_pApptPurpose->GetCurSel();
			if (pRow){
				nApptPurposeID = VarLong(pRow->GetValue(apptPurposeList::apptPurposeID), -1);
				
			}
		}
		else{
			nApptPurposeID = -1;
		}
		
		m_editMaxAppt.GetWindowText(strEditWindowText);
		if (strEditWindowText.IsEmpty()) {
			MessageBox("The maximum number of appointments must be greater than or equal to 0.", "Practice", MB_ICONEXCLAMATION | MB_OK);
			m_editMaxAppt.SetFocus();
			return FALSE;
		}

		
		m_ruleDetail = RuleDetail(-1, nApptTypeID,  nApptPurposeID,  m_nMaxNumAppt);

		return TRUE;

	}NxCatchAll(__FUNCTION__)

		return FALSE;
}

BEGIN_EVENTSINK_MAP(CSchedulerMixRulesAddTypes, CNxDialog)
	ON_EVENT(CSchedulerMixRulesAddTypes, IDC_RULE_APPT_TYPE, 2, CSchedulerMixRulesAddTypes::SelChangedRuleApptType, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

// (s.tullis 2014-12-30 14:39) - Keep Appt Max and Purpose locked unless Appt type is selected
void CSchedulerMixRulesAddTypes::SelChangedRuleApptType(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);
		if (pRow){
			if (lpNewSel == lpOldSel || lpNewSel == NULL ){
				return;
			}

			if (VarLong(pRow->GetValue(appttypeList::appttypeID)) == -1){
				m_pApptPurpose->PutEnabled(VARIANT_FALSE);
				m_editMaxAppt.EnableWindow(TRUE);
			}
			else{
				m_pApptPurpose->PutEnabled(VARIANT_TRUE);
				m_editMaxAppt.EnableWindow(TRUE);
				CString strPurposeFilter;
				strPurposeFilter.Format(" AptPurposeTypeT.AptTypeID = %li ", VarLong(pRow->GetValue(appttypeList::appttypeID)));
				m_pApptPurpose->PutWhereClause(_bstr_t(strPurposeFilter));
				m_pApptPurpose->Requery();
				pRow = m_pApptPurpose->GetNewRow();
				if (pRow){
					pRow->PutValue(apptPurposeList::apptPurposeID, -1);
					pRow->PutValue(appttypeList::appttypeName, "< All Appointment Purposes >");
					m_pApptPurpose->AddRowBefore(pRow, m_pApptPurpose->GetFirstRow());
				}
			}
		}

	}NxCatchAll(__FUNCTION__)
}


// (s.tullis 2014-12-30 14:39) - Make sure we can save.. then End dialog with Defined message.. config will launch again
void CSchedulerMixRulesAddTypes::OnBnClickedRuleDetailSaveAddAnother()
{
	try{
		if (!Save()){
			return;
		}
		EndDialog(IDC_RULE_DETAIL_SAVE_ADD_ANOTHER);
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2014-12-30 14:39) -make sure we can save.. then  return okay
void CSchedulerMixRulesAddTypes::OnBnClickedOk()
{
	try{
		if (!Save()){
			return;
		}
		EndDialog(IDOK);
	}NxCatchAll(__FUNCTION__)
}


