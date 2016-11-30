// CaseHistorySelectSurgeryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CaseHistorySelectSurgeryDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum {
	chssdPreferenceCardID = 0,
	chssdPreferenceCardSelected,	// (j.jones 2009-08-31 14:55) - PLID 35378
	chssdPreferenceCardName
};

enum {
	chssdProviderID = 0,
	chssdProviderName
};

using namespace NXDATALIST2Lib;
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CCaseHistorySelectSurgeryDlg dialog


CCaseHistorySelectSurgeryDlg::CCaseHistorySelectSurgeryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCaseHistorySelectSurgeryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCaseHistorySelectSurgeryDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nPatientID = -1;
	m_nAppointmentID = -1;
}


void CCaseHistorySelectSurgeryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCaseHistorySelectSurgeryDlg)
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_CHECK_FILTER_BY_APPT_PURPOSES, m_checkFilterByApptPurposes);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCaseHistorySelectSurgeryDlg, CNxDialog)
	ON_BN_CLICKED(IDC_CHECK_FILTER_BY_APPT_PURPOSES, OnCheckFilterByApptPurposes)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCaseHistorySelectSurgeryDlg message handlers

BOOL CCaseHistorySelectSurgeryDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-01 10:14) - PLID 29863 - NxIconify buttons
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (c.haag 2007-03-09 11:08) - PLID 25138 - Specify the patient ID for CalcDefaultCaseHistoryProvider.
		// This function used to call GetActivePatientID, so we used to never expect a non-positive value. Now that
		// we can possibly have such values, make sure we don't call CalcDefaultCaseHistoryProvider if that is the 
		// case because it will throw an exception.
		// (j.jones 2009-09-01 09:26) - PLID 17734 - optionally takes in an appointment ID
		long nProviderID = (m_nPatientID > 0) ? CalcDefaultCaseHistoryProvider(m_nPatientID, m_nAppointmentID) : -1;

		m_pProviderCombo = BindNxDataList2Ctrl(this, IDC_CHSS_PROVIDER_COMBO, GetRemoteData(), true);
		// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries
		// (j.jones 2009-08-31 14:49) - PLID 35378 - changed to datalist 2s
		m_pPreferenceCardList = BindNxDataList2Ctrl(this, IDC_PREFERENCE_CARD_MULTISELECT_LIST, GetRemoteData(), false);

		if (-1 != nProviderID) {
			m_pProviderCombo->SetSelByColumn(chssdProviderID, nProviderID);
		}

		// (j.jones 2009-08-31 17:35) - PLID 17734 - if we have an appointment ID,
		// filter by its purposes, for matching preference cards by procedure
		if(m_nAppointmentID != -1) {
			//get the purpose name, but also check and see if we have any matching preference cards
			//(for any provider at all) and if we don't, then don't bother even trying to filter
			_RecordsetPtr rs = CreateParamRecordset("SELECT dbo.GetPurposeString(ID) AS PurposeNames "
				"FROM AppointmentsT "
				"WHERE AppointmentsT.ID = {INT} "
				"AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentPurposeT "
				"	WHERE PurposeID IN (SELECT ProcedureID FROM PreferenceCardProceduresT))", m_nAppointmentID);
			if(!rs->eof) {
				CString strCheckboxLabel;
				strCheckboxLabel.Format("Filter Preference Cards By Appointment Purposes: %s",
					AdoFldString(rs, "PurposeNames", ""));
				GetDlgItem(IDC_CHECK_FILTER_BY_APPT_PURPOSES)->SetWindowText(strCheckboxLabel);
				m_checkFilterByApptPurposes.SetCheck(TRUE);
			}
			else {
				//either the appointment has no purposes, or there are
				//no matching preference cards, so there is no reason to even
				//give them a filter option that does nothing at all
				GetDlgItem(IDC_CHECK_FILTER_BY_APPT_PURPOSES)->ShowWindow(SW_HIDE);
			}
			rs->Close();
		}
		else {
			//no appointment - hide the checkbox entirely
			GetDlgItem(IDC_CHECK_FILTER_BY_APPT_PURPOSES)->ShowWindow(SW_HIDE);
		}

		// (j.jones 2009-08-31 17:41) - PLID 17734 - just call RefilterPreferenceCards
		RefilterPreferenceCards();

		GetDlgItem(IDC_CHSS_PROVIDER_COMBO)->SetFocus();
	}
	NxCatchAll("Error in CCaseHistorySelectSurgeryDlg::OnInitDialog");
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCaseHistorySelectSurgeryDlg::OnOK() 
{
	// (j.jones 2009-08-19 16:32) - PLID 35124 - changed to use Preference Cards, not surgeries

	try {
		
		IRowSettingsPtr pProvRow = m_pProviderCombo->GetCurSel();
		if(pProvRow) {
			m_nProviderID = VarLong(pProvRow->GetValue(chssdProviderID));
		} else {
			MsgBox(MB_ICONINFORMATION|MB_OK, "Please select a provider from the list.");
			return;
		}

		// (j.jones 2009-08-31 14:56) - PLID 35378 - changed to allow multi-selection

		m_arynPreferenceCardIDs.RemoveAll();
		m_strNewCaseHistoryName = "";

		IRowSettingsPtr pRow = m_pPreferenceCardList->GetFirstRow();
		while(pRow) {

			if(VarBool(pRow->GetValue(chssdPreferenceCardSelected))) {
				m_arynPreferenceCardIDs.Add(VarLong(pRow->GetValue(chssdPreferenceCardID)));
				if(!m_strNewCaseHistoryName.IsEmpty()) {
					m_strNewCaseHistoryName += ", ";
				}
				m_strNewCaseHistoryName += VarString(pRow->GetValue(chssdPreferenceCardName), "");
			}

			pRow = pRow->GetNextRow();
		}
		
		// (j.jones 2007-02-20 11:34) - PLID 23994 - a user is now allowed to proceed without
		// selecting a preference card
		// (j.jones 2009-08-31 15:02) - PLID 35378 - rather than give them an option to use no preference card,
		// just prompt them to confirm
		if(m_arynPreferenceCardIDs.GetSize() == 0) {
			if(IDNO == MessageBox("No preference cards are selected. "
				"Would you like to create a case history without using a preference card?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}
		CNxDialog::OnOK();

	} NxCatchAll("CCaseHistorySelectSurgeryDlg::OnOK");
}

BEGIN_EVENTSINK_MAP(CCaseHistorySelectSurgeryDlg, CNxDialog)
	ON_EVENT(CCaseHistorySelectSurgeryDlg, IDC_CHSS_PROVIDER_COMBO, 16, OnSelChosenChssProviderCombo, VTS_DISPATCH)
	ON_EVENT(CCaseHistorySelectSurgeryDlg, IDC_PREFERENCE_CARD_MULTISELECT_LIST, 18, OnRequeryFinishedPreferenceCardMultiselectList, VTS_I2)
END_EVENTSINK_MAP()

void CCaseHistorySelectSurgeryDlg::OnSelChosenChssProviderCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			m_pPreferenceCardList->Clear();
			return;
		}

		// (j.jones 2009-08-31 17:41) - PLID 17734 - just call RefilterPreferenceCards
		RefilterPreferenceCards();
	
	}NxCatchAll("Error filtering the preference card list.");
}

void CCaseHistorySelectSurgeryDlg::SetPatientID(long nPatientID)
{
	//
	// (c.haag 2007-03-09 11:08) - PLID 25138 - Designate the patient ID for calculating
	// the default case history provider
	//
	m_nPatientID = nPatientID;
}

// (j.jones 2009-08-31 17:34) - PLID 17734 - added appt. purpose filter
void CCaseHistorySelectSurgeryDlg::OnCheckFilterByApptPurposes()
{
	try {

		RefilterPreferenceCards();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-08-31 17:41) - PLID 17734 - added central refiltering function
void CCaseHistorySelectSurgeryDlg::RefilterPreferenceCards()
{
	try {

		IRowSettingsPtr pProvRow = m_pProviderCombo->GetCurSel();
		if(pProvRow == NULL) {
			m_pPreferenceCardList->Clear();
			return;
		}

		long nProviderID = VarLong(pProvRow->GetValue(chssdProviderID));

		CString strWhere;
		//-1 or no record means all providers use the surgery, and of course a matching provider ID works too
		// (j.jones 2009-08-24 11:36) - PLID 35124 - changed to PreferenceCardProvidersT
		strWhere.Format("(ID IN (SELECT PreferenceCardID FROM PreferenceCardProvidersT WHERE ProviderID = %li) "
			"OR ID NOT IN (SELECT PreferenceCardID FROM PreferenceCardProvidersT))", nProviderID);
		
		// (j.jones 2009-08-31 17:40) - PLID 17734 - is the appt. filter checked?
		if(m_nAppointmentID != -1 && m_checkFilterByApptPurposes.GetCheck()) {
			CString strWhere2;
			strWhere2.Format(" AND ID IN (SELECT PreferenceCardID FROM PreferenceCardProceduresT "
				"WHERE ProcedureID IN (SELECT PurposeID FROM AppointmentPurposeT "
				"	WHERE AppointmentID = %li))", m_nAppointmentID);
			strWhere += strWhere2;
		}

		m_pPreferenceCardList->PutWhereClause(_bstr_t(strWhere));	
		m_pPreferenceCardList->Requery();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-08-31 17:34) - PLID 17734 - if only one matching card exists, check it off
void CCaseHistorySelectSurgeryDlg::OnRequeryFinishedPreferenceCardMultiselectList(short nFlags)
{
	try {

		//if only one matching card exists, check it off
		if(m_pPreferenceCardList->GetRowCount() == 1) {
			IRowSettingsPtr pRow = m_pPreferenceCardList->GetFirstRow();
			if(pRow) {
				pRow->PutValue(chssdPreferenceCardSelected, g_cvarTrue);
			}
		}

	}NxCatchAll(__FUNCTION__);
}
