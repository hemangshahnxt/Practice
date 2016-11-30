// SecurityGroupsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientsRc.h"
#include "SecurityGroupsDlg.h"
#include "SecurityGroupsConfigDlg.h"
#include "AuditTrail.h"

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



//TES 1/5/2010 - PLID 35774 - Created, inspired by CPatientGroupsDlg.
// CSecurityGroupsDlg dialog

IMPLEMENT_DYNAMIC(CSecurityGroupsDlg, CNxDialog)

// (j.gruber 2011-01-13 15:09) - PLID 40415 - take a variable as to whether update this client's patient toolbar
CSecurityGroupsDlg::CSecurityGroupsDlg(CWnd* pParent, BOOL bGreyList /*=FALSE*/, BOOL bUpdateCurrentClientToolbar /*= FALSE*/)
	: CNxDialog(CSecurityGroupsDlg::IDD, pParent)
{
	m_nPatientID = -1;
	m_bInitialGroupsLoaded = false;
	m_bGreyList = bGreyList;
	m_bUpdateCurrentToolbar = bUpdateCurrentClientToolbar;
}

CSecurityGroupsDlg::~CSecurityGroupsDlg()
{
}

void CSecurityGroupsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	DDX_Control(pDX, IDC_CONFIGURE_GROUPS, m_nxbConfigureGroups);
	DDX_Control(pDX, IDC_SECURITY_GROUPS_LABEL, m_nxsLabel);
}


BEGIN_MESSAGE_MAP(CSecurityGroupsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CSecurityGroupsDlg::OnOK)
	ON_BN_CLICKED(IDC_CONFIGURE_GROUPS, &CSecurityGroupsDlg::OnConfigureGroups)
END_MESSAGE_MAP()

enum SecurityGroupsColumns {
	sgcID = 0,
	sgcChecked = 1,
	sgcName = 2,
};

using namespace NXDATALIST2Lib;
// CSecurityGroupsDlg message handlers
BOOL CSecurityGroupsDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try {

		// (j.gruber 2010-09-09 10:52) - PLID 40414 - cache new preference
		g_propManager.CachePropertiesInBulk("SecurityGroups", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'SecurityGroupSaveOne'"				
				")",
				_Q(GetCurrentUserName()));

		//TES 1/5/2010 - PLID 35774 - Set our NxIconButtons
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);
		m_nxbConfigureGroups.AutoSet(NXB_MODIFY);

		//TES 1/5/2010 - PLID 35774 - Set our background color based on this patient.
		m_pBkg = GetDlgItem(IDC_SECURITY_GROUPS_COLOR)->GetControlUnknown();
		// (b.spivey, May 21, 2012) - PLID 50558 - We use the default patient blue always.
		m_pBkg->PutColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		//TES 1/5/2010 - PLID 35774 - Fill in the label.
		SetDlgItemText(IDC_SECURITY_GROUPS_LABEL, "Security Groups are groups of patients which can be blocked for certain users.  "
			"If a user does not have access to every Security Group of which a patient is a member, they will not be able to access that "
			"patient in the Patients module, or see any information about that patient in the Scheduler module.");

		//TES 1/5/2010 - PLID 35774 - Load our list, based on our patient.
		m_pGroupsList = BindNxDataList2Ctrl(IDC_SECURITY_GROUPS, false);
		CString strFrom;
		strFrom.Format("SecurityGroupsT LEFT JOIN (SELECT * FROM SecurityGroupDetailsT WHERE PatientID = %li) AS SelectedGroupsQ "
			"ON SecurityGroupsT.ID = SelectedGroupsQ.SecurityGroupID", m_nPatientID);
		m_pGroupsList->FromClause = _bstr_t(strFrom);
		m_pGroupsList->Requery();

		// (j.gruber 2010-09-08 11:59) - PLID 40413 - added the patient name to make it clearer who your are setting the groups for.
		CString strPatientName = GetExistingPatientName(m_nPatientID);
		SetWindowTextA("Security Groups for " + strPatientName);


		// (j.gruber 2010-10-26 13:59) - PLID 40416 - if they only have permission to assign patients, then grey out the add/edit button
		if (!(CheckCurrentUserPermissions(bioSecurityGroup, sptWrite, FALSE, FALSE, TRUE, TRUE)) ) {
			GetDlgItem(IDC_CONFIGURE_GROUPS)->EnableWindow(FALSE);
		}
		else {
			GetDlgItem(IDC_CONFIGURE_GROUPS)->EnableWindow(TRUE);			
		}

		// (j.gruber 2010-10-26 13:59) - PLID 40416 - if we don't have permission to add patients, gray out the list and the OK button
		BOOL bPerms = CheckCurrentUserPermissions(bioSecurityGroup, sptDynamic0, 0, 0, TRUE, TRUE);
		if (m_bGreyList || !bPerms) {
			GetDlgItem(IDOK)->EnableWindow(FALSE);
			m_pGroupsList->ReadOnly = true;
		}



	}NxCatchAll(__FUNCTION__);

	return TRUE;
}
void CSecurityGroupsDlg::OnOK()
{
	try {

		// (j.gruber 2010-09-09 10:53) - PLID 40414 - if the preference is set, make sure they've checked at least one
		long nCheck = GetRemotePropertyInt("SecurityGroupSaveOne", 0, 0, "<None>");

		if (nCheck != 0) {
			long nCount = 0;
			IRowSettingsPtr pRow = m_pGroupsList->GetFirstRow();
			while(pRow) {
				if(VarBool(pRow->GetValue(sgcChecked))) {
					nCount++;
				}
			
				pRow = pRow->GetNextRow();
			}

			if (nCount < 1) {
				MsgBox("Please select at least one security group before saving.");
				return;
			}
		}


		//TES 1/5/2010 - PLID 35774 - Track the names of the groups that are checked now
		CStringArray saNewGroups;
		CString strSql;
		//TES 1/5/2010 - PLID 35774 - Clear out old data.
		AddStatementToSqlBatch(strSql, "DELETE FROM SecurityGroupDetailsT WHERE PatientID = %li", m_nPatientID);
		//TES 1/5/2010 - PLID 35774 - Add all checked groups.
		IRowSettingsPtr pRow = m_pGroupsList->GetFirstRow();
		while(pRow) {
			if(VarBool(pRow->GetValue(sgcChecked))) {
				AddStatementToSqlBatch(strSql, "INSERT INTO SecurityGroupDetailsT (SecurityGroupID, PatientID) VALUES (%li, %li)",
					VarLong(pRow->GetValue(sgcID)), m_nPatientID);
				saNewGroups.Add(VarString(pRow->GetValue(sgcName)));
			}
			pRow = pRow->GetNextRow();
		}
		ExecuteSqlBatch(strSql);

		//TES 1/5/2010 - PLID 35774 - Do we have the same groups selected that we started with?
		bool bArraysMatched = true;
		if(saNewGroups.GetSize() != m_saInitialGroups.GetSize()) {
			bArraysMatched = false;
		}
		for(int i = 0; i < m_saInitialGroups.GetSize() && bArraysMatched; i++) {
			if(!IsStringInStringArray(m_saInitialGroups[i], &saNewGroups)) {
				bArraysMatched = false;
			}
		}
		if(!bArraysMatched) {
			CString strNew = GenerateDelimitedListFromStringArray(saNewGroups, ", ");
			//TES 1/5/2010 - PLID 35774 - Nope, so audit the change.
			AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiPatientSecurityGroups, m_nPatientID, 
				GenerateDelimitedListFromStringArray(m_saInitialGroups, ", "), strNew, aepHigh);

			// (j.gruber 2010-10-04 14:41) - PLID 40415 - update our own toolbar quickly, then the below will update the others
			// (j.gruber 2011-01-13 15:10) - PLID 40415 - only update if we tell it to
			if (m_bUpdateCurrentToolbar) {
				GetMainFrame()->m_patToolBar.SetCurrentlySelectedValue(GetMainFrame()->m_patToolBar.GetSecurityGroupColumn(), _bstr_t(strNew));
			}

			//TES 1/18/2010 - PLID 36895 - We also need to update the toolbar, this patient's demographics may now be shown/hidden.
			CClient::RefreshTable(NetUtils::PatCombo, m_nPatientID);
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CSecurityGroupsDlg::OnConfigureGroups()
{
	try {
		//TES 1/5/2010 - PLID 35774 - Just launch the dialog.
		// (j.gruber 2010-10-26 14:07) - PLID 40416 - check to see if they have permission
		if (CheckCurrentUserPermissions(bioSecurityGroup, sptWrite, FALSE, FALSE, FALSE, FALSE) ) {
			CSecurityGroupsConfigDlg dlg(this);
			if(dlg.DoModal() == IDOK) {
				m_pGroupsList->Requery();
			}
		}

	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CSecurityGroupsDlg, CNxDialog)
	ON_EVENT(CSecurityGroupsDlg, IDC_SECURITY_GROUPS, 18, CSecurityGroupsDlg::OnRequeryFinishedSecurityGroups, VTS_I2)
END_EVENTSINK_MAP()

void CSecurityGroupsDlg::OnRequeryFinishedSecurityGroups(short nFlags)
{
	try {
		//TES 1/5/2010 - PLID 35774 - For auditing, remember which groups were checked initially.
		if(!m_bInitialGroupsLoaded) {
			IRowSettingsPtr pRow = m_pGroupsList->GetFirstRow();
			while(pRow) {
				if(VarBool(pRow->GetValue(sgcChecked))) {
					m_saInitialGroups.Add(VarString(pRow->GetValue(sgcName)));
				}
				pRow = pRow->GetNextRow();
			}
			m_bInitialGroupsLoaded = true;
		}
	}NxCatchAll(__FUNCTION__);
}
