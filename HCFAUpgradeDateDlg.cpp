// HCFAUpgradeDateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "HCFAUpgradeDateDlg.h"
#include "AuditTrail.h"
#include "GlobalFinancialUtils.h"

// (j.jones 2013-08-02 15:23) - PLID 57805 - created

// CHCFAUpgradeDateDlg dialog

using namespace ADODB;

IMPLEMENT_DYNAMIC(CHCFAUpgradeDateDlg, CNxDialog)

CHCFAUpgradeDateDlg::CHCFAUpgradeDateDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHCFAUpgradeDateDlg::IDD, pParent)
{
	m_bFromHCFAGroup = false;
	m_nID = -1;
	m_dtOldUpgradeDate = g_cdtDefaultHCFAUpgrade;	//defaults to 4/1/2014
	m_bOldAllowed = false;
}

CHCFAUpgradeDateDlg::~CHCFAUpgradeDateDlg()
{
}

void CHCFAUpgradeDateDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_HCFA_UPGRADE_DATE_BKG, m_bkg);
	DDX_Control(pDX, IDC_HCFA_UPGRADE_DATE, m_dtUpgradeDate);
	DDX_Control(pDX, IDC_CHECK_OLD_HCFA_ALLOWED, m_checkOldFormAllowed);
	DDX_Control(pDX, IDC_HCFA_UPGRADE_LABEL, m_nxstaticLabel);
}


BEGIN_MESSAGE_MAP(CHCFAUpgradeDateDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
END_MESSAGE_MAP()


// CHCFAUpgradeDateDlg message handlers

BOOL CHCFAUpgradeDateDlg::OnInitDialog() 
{	
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_bkg.SetColor(m_nColor);

		CString strLabel;

		//we load the screen differently based on whether we're editing
		//one insurance company, or about to mass-update a HCFA group
		if(m_bFromHCFAGroup) {
			//load some useful information from the HCFA group
			_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 2 HCFASetupT.Name, InsuranceCoT.HCFAUpgradeDate, InsuranceCoT.HCFAOldFormAllowed "
				"FROM HCFASetupT "
				"INNER JOIN InsuranceCoT ON HCFASetupT.ID = InsuranceCoT.HCFASetupGroupID "
				"WHERE HCFASetupT.ID = {INT} "
				"GROUP BY HCFASetupT.Name, InsuranceCoT.HCFAUpgradeDate, InsuranceCoT.HCFAOldFormAllowed "
				"ORDER BY InsuranceCoT.HCFAUpgradeDate, InsuranceCoT.HCFAOldFormAllowed", m_nID);
			if(rs->eof) {
				//epic fail
				AfxMessageBox("The HCFA group you were editing is no longer available. It may have been deleted.\n\n"
					"If this error persists, please contact NexTech Technical Support.");
				CNxDialog::OnCancel();
				return TRUE;
			}
			else {
				//get the group name, and the nearest upgrade date
				m_strName = VarString(rs->Fields->Item["Name"]->Value);
				m_dtOldUpgradeDate = VarDateTime(rs->Fields->Item["HCFAUpgradeDate"]->Value);
				m_bOldAllowed = VarBool(rs->Fields->Item["HCFAOldFormAllowed"]->Value) ? true : false;

				strLabel.Format("Changes to this screen will update all insurance companies in the %s group to "
					"use the new HCFA form beginning on the date selected below.", m_strName);

				//If two records are returned, it means that there are companies in this group with
				//differing values. Otherwise with one record it would mean that all companies in
				//the group have the same settings.
				rs->MoveNext();
				if(!rs->eof) {
					strLabel += "\nCurrently, not every company in this group has the same settings.";
				}
			}

			rs->Close();
		}
		else {
			//load from the insurance company
			_RecordsetPtr rs = CreateParamRecordset("SELECT Name, HCFAUpgradeDate, HCFAOldFormAllowed FROM InsuranceCoT WHERE PersonID = {INT}", m_nID);
			if(rs->eof) {
				//epic fail
				AfxMessageBox("The insurance company you were editing is no longer available. It may have been deleted.\n\n"
					"If this error persists, please contact NexTech Technical Support.");
				CNxDialog::OnCancel();
				return TRUE;
			}
			else {
				m_strName = VarString(rs->Fields->Item["Name"]->Value);
				m_dtOldUpgradeDate = VarDateTime(rs->Fields->Item["HCFAUpgradeDate"]->Value);
				m_bOldAllowed = VarBool(rs->Fields->Item["HCFAOldFormAllowed"]->Value) ? true : false;

				strLabel.Format("The insurance company %s will use the new HCFA form beginning on the date selected below.", m_strName);
			}
			rs->Close();
		}

		//update the screen
		m_nxstaticLabel.SetWindowText(strLabel);
		m_dtUpgradeDate.SetValue(m_dtOldUpgradeDate);
		m_checkOldFormAllowed.SetCheck(m_bOldAllowed);

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

//if bFromHCFAGroup is true, nID is a HCFASetupT.ID,
//if it is false, nID is InsuranceCoT.PersonID
int CHCFAUpgradeDateDlg::DoModal(long nID, bool bFromHCFAGroup, OLE_COLOR nColor)
{
	try {

		m_nID = nID;
		m_bFromHCFAGroup = bFromHCFAGroup;
		m_nColor = nColor;

		return CNxDialog::DoModal();

	}NxCatchAll(__FUNCTION__);	

	return IDCANCEL;
}

void CHCFAUpgradeDateDlg::OnOK()
{
	long nAuditTransactionID = -1;

	try {

		CWaitCursor pWait;

		COleDateTime dtUpgradeDate = m_dtUpgradeDate.GetValue();
		bool bOldFormAllowed = m_checkOldFormAllowed.GetCheck() ? true : false;
		if(dtUpgradeDate.GetStatus() != COleDateTime::valid || dtUpgradeDate < g_cdtSqlMin) {
			AfxMessageBox("The date you have entered is invalid. Please correct the date before saving.");
			return;
		}

		CString strSqlBatch;
		CNxParamSqlArray aryParams;

		if(m_bFromHCFAGroup) {
			//update all insurance companies in this group
			_RecordsetPtr rs = CreateParamRecordset("SELECT PersonID, Name, HCFAUpgradeDate, HCFAOldFormAllowed "
				"FROM InsuranceCoT "
				"WHERE HCFASetupGroupID = {INT} "
				"ORDER BY Name", m_nID);
			while(!rs->eof) {
				long nID = VarLong(rs->Fields->Item["PersonID"]->Value);
				CString strName = VarString(rs->Fields->Item["Name"]->Value);
				COleDateTime dtOldUpgradeDate = VarDateTime(rs->Fields->Item["HCFAUpgradeDate"]->Value);
				bool bOldAllowed = VarBool(rs->Fields->Item["HCFAOldFormAllowed"]->Value) ? true : false;

				//update this company, nothing will happen if values did not change
				TryUpdateInsuranceCompany(strSqlBatch, aryParams, nAuditTransactionID, nID, strName,
					dtOldUpgradeDate, dtUpgradeDate, bOldAllowed, bOldFormAllowed);

				rs->MoveNext();
			}
			rs->Close();
		}
		else {
			//update just this one company, nothing will happen if values did not change
			TryUpdateInsuranceCompany(strSqlBatch, aryParams, nAuditTransactionID, m_nID, m_strName,
				m_dtOldUpgradeDate, dtUpgradeDate, m_bOldAllowed, bOldFormAllowed);
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, aryParams);
			if(nAuditTransactionID != -1) {
				CommitAuditTransaction(nAuditTransactionID);
				nAuditTransactionID = -1;
			}
		}

		CNxDialog::OnOK();

	}NxCatchAllCall(__FUNCTION__,
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	)
}

void CHCFAUpgradeDateDlg::TryUpdateInsuranceCompany(CString &strSqlBatch, CNxParamSqlArray &aryParams, long &nAuditTransactionID,
												 const long nInsuranceCoID, const CString strInsuranceCoName,
												 const COleDateTime dtOldUpgradeDate, const COleDateTime dtNewUpgradeDate,
												 const bool bOldFormAllowed, const bool bNewFormAllowed)
{
	if(dtOldUpgradeDate != dtNewUpgradeDate || bOldFormAllowed != bNewFormAllowed) {

		AddParamStatementToSqlBatch(strSqlBatch, aryParams, "UPDATE InsuranceCoT SET HCFAUpgradeDate = {OLEDATETIME}, HCFAOldFormAllowed = {BIT} "
			"WHERE PersonID = {INT}", dtNewUpgradeDate, bNewFormAllowed, nInsuranceCoID);

		if(nAuditTransactionID == -1) {
			nAuditTransactionID = BeginAuditTransaction();
		}

		CString strOldValue, strNewValue;

		if(dtOldUpgradeDate != dtNewUpgradeDate) {
			strOldValue.Format("Use the 02-12 HCFA form beginning on: %s", FormatDateTimeForInterface(dtOldUpgradeDate, NULL, dtoDate));
			strNewValue.Format("Use the 02-12 HCFA form beginning on: %s", FormatDateTimeForInterface(dtNewUpgradeDate, NULL, dtoDate));
			AuditEvent(-1, strInsuranceCoName, nAuditTransactionID, aeiInsCoHCFAUpgradeDate, nInsuranceCoID, strOldValue, strNewValue, aepMedium, aetChanged);
		}
		if(bOldFormAllowed != bNewFormAllowed) {
			if(bOldFormAllowed) {
				strOldValue = "Use the 08-05 HCFA form on old service dates.";
			}
			else {
				strOldValue = "Do not use the 08-05 HCFA form on old service dates.";
			}
			if(bNewFormAllowed) {
				strNewValue = "Use the 08-05 HCFA form on old service dates.";
			}
			else {
				strNewValue = "Do not use the 08-05 HCFA form on old service dates.";
			}
			AuditEvent(-1, strInsuranceCoName, nAuditTransactionID, aeiInsCoHCFAOldFormAllowed, nInsuranceCoID, strOldValue, strNewValue, aepMedium, aetChanged);
		}
	}
}