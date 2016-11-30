// ViewSchedulerMixRulesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SchedulerRc.h"
#include "ViewSchedulerMixRulesDlg.h"
#include "afxdialogex.h"
#include "NxAPI.h"

//TES 11/18/2014 - PLID 64121 - Created
// CViewSchedulerMixRulesDlg dialog

IMPLEMENT_DYNAMIC(CViewSchedulerMixRulesDlg, CNxDialog)

CViewSchedulerMixRulesDlg::CViewSchedulerMixRulesDlg(CWnd* pParent /*=NULL*/)
: CNxDialog(CViewSchedulerMixRulesDlg::IDD, pParent)
{

}

CViewSchedulerMixRulesDlg::~CViewSchedulerMixRulesDlg()
{
}

void CViewSchedulerMixRulesDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	DDX_Control(pDX, IDC_RULE_DATE, m_dtpRuleDate);
	DDX_Control(pDX, IDC_DATE_LEFT, m_btnDateLeft);
	DDX_Control(pDX, IDC_DATE_RIGHT, m_btnDateRight);
}


BEGIN_MESSAGE_MAP(CViewSchedulerMixRulesDlg, CNxDialog)
	ON_BN_CLICKED(IDC_GO_TO_SCHEDULE, &CViewSchedulerMixRulesDlg::OnBnClickedGoToSchedule)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_RULE_DATE, &CViewSchedulerMixRulesDlg::OnDtnDatetimechangeRuleDate)
	ON_BN_CLICKED(IDC_DATE_LEFT, &CViewSchedulerMixRulesDlg::OnBnClickedDateLeft)
	ON_BN_CLICKED(IDC_DATE_RIGHT, &CViewSchedulerMixRulesDlg::OnBnClickedDateRight)
END_MESSAGE_MAP()

enum ResourceListColumns {
	rlcID = 0,
	rlcName = 1,
};

//TES 11/19/2014 - PLID 64122
enum RuleColumns {
	rcID = 0,
	rcName = 1,
	rcAvailAppts = 2,
	rcMaxAppts = 3,
};

using namespace NXDATALIST2Lib;

//TES 11/24/2014 - PLID 64273
#define OVER_SCHEDULED_COLOR RGB (255,0,0)

// CViewSchedulerMixRulesDlg message handlers
BOOL CViewSchedulerMixRulesDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnDateLeft.AutoSet(NXB_LEFT);
		m_btnDateRight.AutoSet(NXB_RIGHT);
		m_pResourceList = BindNxDataList2Ctrl(IDC_RULE_RESOURCE);
		//TES 11/19/2014 - PLID 64122
		m_pRuleList = BindNxDataList2Ctrl(IDC_RULE_LIST, false);

		//TES 11/18/2014 - PLID 64123 - Initialize to the resource/date we were given.
		m_dtpRuleDate.SetValue(m_dtInitialDate);
		m_pResourceList->SetSelByColumn(rlcID, m_nInitialResourceID);

		//TES 11/18/2014 - PLID 64124 - Set our selected resource/date
		m_dtNewDate = m_dtInitialDate;
		m_nNewResourceID = m_nInitialResourceID;

		LoadRules();
		

	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CViewSchedulerMixRulesDlg::OnBnClickedGoToSchedule()
{
	try {
		//TES 11/18/2014 - PLID 64124 - Set the selected date/resource
		IRowSettingsPtr pRow = m_pResourceList->CurSel;
		if (pRow == NULL) {
			MsgBox("Please select a resource to schedule for");
			return;
		}
		m_nNewResourceID = VarLong(pRow->GetValue(rlcID));
		m_dtNewDate = VarDateTime(m_dtpRuleDate.GetValue());
		OnOK();
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CViewSchedulerMixRulesDlg, CNxDialog)
	ON_EVENT(CViewSchedulerMixRulesDlg, IDC_RULE_RESOURCE, 16, CViewSchedulerMixRulesDlg::SelChosenRuleResource, VTS_DISPATCH)
END_EVENTSINK_MAP()


void CViewSchedulerMixRulesDlg::SelChosenRuleResource(LPDISPATCH lpRow)
{
	try {
		LoadRules();
	}NxCatchAll(__FUNCTION__);
}

void CViewSchedulerMixRulesDlg::LoadRules()
{
	m_pRuleList->Clear();
	IRowSettingsPtr pRow = m_pResourceList->CurSel;
	if (pRow == NULL) {
		return;
	}
	//TES 11/19/2014 - PLID 64122 - Now we need to load the rules, which we get from the API.
	NexTech_Accessor::_GetSchedulingMixRulesInputPtr pInput(__uuidof(NexTech_Accessor::GetSchedulingMixRulesInput));
	pInput->date = VarDateTime(m_dtpRuleDate.GetValue());
	pInput->ResourceID = _bstr_t(AsString(pRow->GetValue(rlcID)));
	NexTech_Accessor::_SchedulingMixRulesPtr pRules = GetAPI()->GetSchedulingMixRules(GetAPISubkey(), GetAPILoginToken(), pInput);

	//TES 11/19/2014 - PLID 64122 - Read through the results
	Nx::SafeArray<IUnknown *> saResults = pRules->Results;
	for each(NexTech_Accessor::_SchedulingMixRulePtr rule in saResults) {
		//TES 11/19/2014 - PLID 64122 - Add the parent row for the rule
		IRowSettingsPtr pRow = m_pRuleList->GetNewRow();
		pRow->PutValue(rcID, rule->ruleID);
		pRow->PutValue(rcName, rule->RuleName);
		m_pRuleList->AddRowAtEnd(pRow, NULL);
		Nx::SafeArray<IUnknown *> saDetails = rule->details;
		for each(NexTech_Accessor::_SchedulingMixRuleDetailPtr detail in saDetails) {
			//TES 11/19/2014 - PLID 64122 - Add a child row for each detail
			IRowSettingsPtr pDetailRow = m_pRuleList->GetNewRow();
			pDetailRow->PutValue(rcID, detail->detailID);
			CString strType = VarString(detail->AptTypeName);
			if (strType.IsEmpty()) strType = "< All Types >";
			CString strPurpose = VarString(detail->AptPurposeName);
			if (strPurpose.IsEmpty()) strPurpose = "< All Purposes >";
			pDetailRow->PutValue(rcName, _bstr_t(strType + " - " + strPurpose));
			pDetailRow->PutValue(rcAvailAppts, detail->ApptsRemaining);
			pDetailRow->PutValue(rcMaxAppts, detail->MaxAppts);
			//TES 11/24/2014 - PLID 64273 - If the rule is overscheduled, color it
			if (detail->ApptsRemaining < 0) {
				pDetailRow->PutCellForeColor(rcName, OVER_SCHEDULED_COLOR);
				pDetailRow->PutCellForeColor(rcAvailAppts, OVER_SCHEDULED_COLOR);
			}
			m_pRuleList->AddRowAtEnd(pDetailRow, pRow);
		}
		//TES 11/19/2014 - PLID 64122 - Now put in our totals
		//TES 12/16/2014 - PLID 64122 - Actually we don't put totals in the top level, so make sure it's expanded
		pRow->PutValue(rcAvailAppts, g_cvarNull);
		pRow->PutValue(rcMaxAppts, g_cvarNull);
		pRow->Expanded = g_cvarTrue;

	}

	//TES 12/29/2014 - PLID 64122 - If there were no rules, add a "<No Rules>" row
	if (m_pRuleList->GetRowCount() == 0) {
		IRowSettingsPtr pRow = m_pRuleList->GetNewRow();
		pRow->PutValue(rcID, g_cvarNull);
		pRow->PutValue(rcName, _bstr_t("<No Rules>"));
		pRow->PutValue(rcAvailAppts, g_cvarNull);
		pRow->PutValue(rcMaxAppts, g_cvarNull);
		m_pRuleList->AddRowAtEnd(pRow, NULL);
	}
}

void CViewSchedulerMixRulesDlg::OnDtnDatetimechangeRuleDate(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	try {
		LoadRules();
	}NxCatchAll(__FUNCTION__);
	*pResult = 0;
}


void CViewSchedulerMixRulesDlg::OnBnClickedDateLeft()
{
	try {
		COleDateTime dt = VarDateTime(m_dtpRuleDate.GetValue());
		dt -= COleDateTimeSpan(1, 0, 0, 0);
		m_dtpRuleDate.SetValue((_variant_t)dt);
		LoadRules();
	}NxCatchAll(__FUNCTION__);
}


void CViewSchedulerMixRulesDlg::OnBnClickedDateRight()
{
	try {
		COleDateTime dt = VarDateTime(m_dtpRuleDate.GetValue());
		dt += COleDateTimeSpan(1, 0, 0, 0);
		m_dtpRuleDate.SetValue((_variant_t)dt);
		LoadRules();
	}NxCatchAll(__FUNCTION__);
}
