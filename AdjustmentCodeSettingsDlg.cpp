// ConfigERemitAdjCodesToIgnoreDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdjustmentCodeSettingsDlg.h"
#include "GlobalFinancialUtils.h"
#include "AuditTrail.h"

// (r.gonet 2016-04-18) - NX-100162 - Renamed dialog class from CConfigERemitAdjCodesToIgnoreDlg to CAdjustmentCodesSettingsDlg
// CAdjustmentCodeSettingsDlg dialog

// (j.jones 2008-11-24 10:09) - PLID 32075 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum CodeListColumns {

	clcID = 0,
	clcGroupCodeID,
	clcReasonCodeID,
};

CAdjustmentCodeSettingsDlg::CAdjustmentCodeSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdjustmentCodeSettingsDlg::IDD, pParent)
{
	m_bInfoChanged = FALSE;
	m_nPRGroupCodeID = -1;
}

void CAdjustmentCodeSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ADD_ADJ_CODE, m_btnAddIgnoredAdjCode);
	DDX_Control(pDX, IDC_REMOVE_ADJ_CODE, m_btnRemoveIgnoredAdjCode);
	DDX_Control(pDX, IDC_EREMIT_ZERO_DOLLAR_ADJ_LABEL, m_nxstaticZeroAdjLabel);
	// (j.jones 2011-04-04 15:34) - PLID 42571 - added ability to ignore all secondary adjustments
	DDX_Control(pDX, IDC_CHECK_IGNORE_SECONDARY_ADJ, m_checkIgnoreSecondaryAdjs);
	DDX_Control(pDX, IDC_ALLOW_ALL_NEGATIVE_ADJUSTMENTS_CHECK, m_checkAllowAllNegativeAdjustmentsToBePosted);
	DDX_Control(pDX, IDC_ADD_ALLOW_NEGATIVE_ADJ_CODE_BUTTON, m_btnAddAllowNegativeAdjCode);
	DDX_Control(pDX, IDC_REMOVE_ALLOW_NEGATIVE_ADJ_CODE_BUTTON, m_btnRemoveAllowNegativeAdjCode);
}


BEGIN_MESSAGE_MAP(CAdjustmentCodeSettingsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDC_ADD_ADJ_CODE, OnAddIgnoredAdjCode)
	ON_BN_CLICKED(IDC_REMOVE_ADJ_CODE, OnRemoveIgnoredAdjCode)
	ON_BN_CLICKED(IDC_ALLOW_ALL_NEGATIVE_ADJUSTMENTS_CHECK, &CAdjustmentCodeSettingsDlg::OnBnClickedAllowAllNegativeAdjustmentsCheck)
	ON_BN_CLICKED(IDC_ADD_ALLOW_NEGATIVE_ADJ_CODE_BUTTON, &CAdjustmentCodeSettingsDlg::OnBnClickedAddAllowNegativeAdjCode)
	ON_BN_CLICKED(IDC_REMOVE_ALLOW_NEGATIVE_ADJ_CODE_BUTTON, &CAdjustmentCodeSettingsDlg::OnBnClickedRemoveAllowNegativeAdjCode)
END_MESSAGE_MAP()

// CAdjustmentCodeSettingsDlg message handlers

BOOL CAdjustmentCodeSettingsDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnAddIgnoredAdjCode.AutoSet(NXB_NEW);
		m_btnRemoveIgnoredAdjCode.AutoSet(NXB_DELETE);
		// (r.gonet 2016-04-18) - NX-100162 - Autoset the allow negative buttons.
		m_btnAddAllowNegativeAdjCode.AutoSet(NXB_NEW);
		m_btnRemoveAllowNegativeAdjCode.AutoSet(NXB_DELETE);

		// (j.jones 2011-04-04 15:34) - PLID 42571 - added ConfigRT caching
		// (r.gonet 2016-04-18) - NX-100162 - Added ERemit_AllowAllNegativeAdjustmentsToBePosted
		g_propManager.CachePropertiesInBulk("CAdjustmentCodeSettingsDlg", propNumber,
				"(Username = '<None>' OR Username = '%s') AND ("
				"Name = 'ERemit_IgnoreSecondaryAdjs' "
				"OR Name = 'ERemit_AllowAllNegativeAdjustmentsToBePosted' "
				")",
				_Q(GetCurrentUserName()));

		//update the zero dollar adjustment label
		//this preference is bulk cached in the calling EOBDlg
		BOOL bAllowZeroDollarAdjustments = GetRemotePropertyInt("ERemitAllowZeroDollarAdjustments", 0, 0, "<None>", true) == 1;
		if(bAllowZeroDollarAdjustments) {
			m_nxstaticZeroAdjLabel.SetWindowTextA("Your preferences are currently configured to allow zero dollar adjustments.");
		}
		else {
			m_nxstaticZeroAdjLabel.SetWindowTextA("Your preferences are currently configured to skip zero dollar adjustments.");
		}

		m_pIgnoredAdjCodesList = BindNxDataList2Ctrl(IDC_EREMIT_CODES_TO_SKIP_LIST, false);
		// (r.gonet 2016-04-18) - NX-100162 - Added binding for the allow negative codes list.
		m_pAllowNegativeAdjCodesList = BindNxDataList2Ctrl(IDC_ALLOW_NEGATIVE_ADJ_CODES_LIST, false);

		// (j.jones 2010-09-23 12:49) - PLID 40653 - these are now stored in data
		//insert a blank line
		CString strGroupCodesSql = "SELECT -1 AS ID, '<Select a Group Code>' AS Name "
			"UNION SELECT ID, Code + ' - ' + Convert(nvarchar(4000), Description) AS Name FROM AdjustmentCodesT WHERE Type = 1 ORDER BY Name";
		//for reason codes, we want to display all the possible codes, even expired ones
		CString strReasonCodesSql = "SELECT -1 AS ID, '<Select a Reason Code>' AS Name "
			"UNION SELECT ID, Code + ' - ' + Convert(nvarchar(4000), Description) AS Name FROM AdjustmentCodesT WHERE Type = 2 ORDER BY Name";

		m_pIgnoredAdjCodesList->GetColumn(clcGroupCodeID)->PutComboSource((LPCTSTR)strGroupCodesSql);
		m_pIgnoredAdjCodesList->GetColumn(clcReasonCodeID)->PutComboSource((LPCTSTR)strReasonCodesSql);
		m_pIgnoredAdjCodesList->Requery();
		
		// (r.gonet 2016-04-18) - NX-100162 - Use the adjustment codes sql as the combo source for the allow negative list embedded combos as well.
		m_pAllowNegativeAdjCodesList->GetColumn(clcGroupCodeID)->PutComboSource((LPCTSTR)strGroupCodesSql);
		m_pAllowNegativeAdjCodesList->GetColumn(clcReasonCodeID)->PutComboSource((LPCTSTR)strReasonCodesSql);
		m_pAllowNegativeAdjCodesList->Requery();

		// (j.jones 2010-09-23 14:31) - PLID 40653 - cache the ID for the PR group code
		m_nPRGroupCodeID = -1;
		_RecordsetPtr rs = CreateRecordset("SELECT ID FROM AdjustmentCodesT WHERE Type = 1 AND Code = 'PR'");
		if(!rs->eof) {
			m_nPRGroupCodeID = AdoFldLong(rs, "ID");
		}
		rs->Close();

		// (j.jones 2011-04-04 15:34) - PLID 42571 - added ability to ignore all secondary adjustments
		// (j.jones 2012-06-28 08:48) - PLID 51240 - changed the default to be ON
		m_ignoredAdjCodeModifications.bOldIgnoreSecondaryAdjs = (GetRemotePropertyInt("ERemit_IgnoreSecondaryAdjs", 1, 0, "<None>", true) == 1);
		m_checkIgnoreSecondaryAdjs.SetCheck(m_ignoredAdjCodeModifications.bOldIgnoreSecondaryAdjs);

		// (r.gonet 2016-04-18) - NX-100162 - If the user has configured Practice to always allow negative adjustments to be
		// posted, then check the box here. Otherwise, load whatever codes they had configured to allow negative adjustment posting
		// per code level.
		m_allowNegativePostingAdjCodeModifications.bOldAllowAllNegativeAdjustmentsToBePosted = (GetRemotePropertyInt("ERemit_AllowAllNegativeAdjustmentsToBePosted", FALSE, 0, "<None>", true) == 1);
		m_checkAllowAllNegativeAdjustmentsToBePosted.SetCheck(m_allowNegativePostingAdjCodeModifications.bOldAllowAllNegativeAdjustmentsToBePosted);

		// (r.gonet 2016-04-18) - NX-100162 - Ensure the buttons and lists are enabled or disabled if they need to be.
		EnsureControls();

	}NxCatchAll(__FUNCTION__);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdjustmentCodeSettingsDlg::OnOk()
{
	try {
		// (r.gonet 2016-04-18) - NX-100162 - Refactored this function a bit to split up the validation from
		// the saving and encapsulate the different components of saving. Had I not done so, adding in saving for the
		// Allow Negative Adjustment Posting codes would have been trying to add to a hot mess and would have
		// been more risky.

		// (r.gonet 2016-04-18) - NX-100162 - Perform the validation of the two lists.
		if (!ValidateIgnoredAdjCodes()) {
			return;
		}
		if(!ValidateAllowNegativePostingAdjCodes()) { 
			return;
		}

		// (r.gonet 2016-04-18) - NX-100162 - Now create the save SQL.
		CParamSqlBatch sqlSaveBatch;
		// (b.eyers 2015-03-30) - PLID 42918
		// (r.gonet 2016-04-18) - NX-100162 - Turned the auditing into an object for encapsulation and
		// and RAII.
		CAuditTransaction auditTransaction;

		DeleteRemovedIgnoredAdjCodes(sqlSaveBatch, auditTransaction);
		UpdateChangedIgnoredAdjCodes(sqlSaveBatch, auditTransaction);
		CreateNewIgnoredAdjCodes(sqlSaveBatch, auditTransaction);

		DeleteRemovedAllowNegativePostingAdjCodes(sqlSaveBatch, auditTransaction);
		UpdateChangedAllowNegativePostingAdjCodes(sqlSaveBatch, auditTransaction);
		CreateNewAllowNegativePostingAdjCodes(sqlSaveBatch, auditTransaction);

		// (r.gonet 2016-04-18) - NX-100162 - Execute if we have stuff to save.
		if(!sqlSaveBatch.IsEmpty()) {
			// (e.lally 2009-06-21) PLID 34679 - Leave as execute batch
			sqlSaveBatch.Execute(GetRemoteData());
		
			//track that we changed something
			m_bInfoChanged = TRUE;
		}

		// (j.jones 2011-04-04 15:34) - PLID 42571 - added ability to ignore all secondary adjustments
		BOOL bIgnoreSecondaryAdjs = m_checkIgnoreSecondaryAdjs.GetCheck();
		SetRemotePropertyInt("ERemit_IgnoreSecondaryAdjs", bIgnoreSecondaryAdjs ? 1 : 0, 0, "<None>");

		if(m_ignoredAdjCodeModifications.bOldIgnoreSecondaryAdjs != bIgnoreSecondaryAdjs) {
			//track that we changed something
			m_bInfoChanged = TRUE;
		}

		// (r.gonet 2016-04-18) - NX-100162 - Save the Allow All setting and audit it if it is different.
		// I choose to audit it because if it is enabled, it will clear out all adjustment codes configured to
		// allow negative adjustment posting.
		BOOL bAllowAllNegativeAdjustmentsToBePosted = m_checkAllowAllNegativeAdjustmentsToBePosted.GetCheck() == BST_CHECKED;
		SetRemotePropertyInt("ERemit_AllowAllNegativeAdjustmentsToBePosted", bAllowAllNegativeAdjustmentsToBePosted ? TRUE : FALSE, 0, "<None>");

		if (m_allowNegativePostingAdjCodeModifications.bOldAllowAllNegativeAdjustmentsToBePosted != bAllowAllNegativeAdjustmentsToBePosted) {
			CString strOld = m_allowNegativePostingAdjCodeModifications.bOldAllowAllNegativeAdjustmentsToBePosted ? "Enabled" : "Disabled";
			CString strNew = bAllowAllNegativeAdjustmentsToBePosted ? "Enabled" : "Disabled";
			AuditEvent(-1, "", auditTransaction, aeiERemitAllowAllNegativeAdjPosting, -1, strOld, strNew, 2, aetChanged);

			//track that we changed something
			m_bInfoChanged = TRUE;
		}

		// (b.eyers 2015-03-30) - PLID 42918
		// (r.gonet 2016-04-18) - NX-100162 - Turned into an object.
		auditTransaction.Commit();

		CNxDialog::OnOK();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-04-18) - NX-100162 - Encapsulation of the validation of the ignored adjustment codes.
bool CAdjustmentCodeSettingsDlg::ValidateIgnoredAdjCodes()
{
	//Disallow saving if -1 is selected in either column
	//Disallow saving duplicates
	{
		IRowSettingsPtr pRow = m_pIgnoredAdjCodesList->GetFirstRow();
		while (pRow) {

			// (j.jones 2010-09-23 12:49) - PLID 40653 - these are now IDs
			long nGroupCodeID = VarLong(pRow->GetValue(clcGroupCodeID), -1);
			long nReasonCodeID = VarLong(pRow->GetValue(clcReasonCodeID), -1);

			if (nGroupCodeID == -1) {
				// (r.gonet 2016-04-18) - NX-100162 - Edited the text a little since this dialog now has two lists of codes.
				AfxMessageBox("At least one entry in the adjustment codes to ignore list has no Group Code selected.\n"
					"All entries must have both a Group Code and a Reason Code selected.");
				return false;
			}

			if (nReasonCodeID == -1) {
				// (r.gonet 2016-04-18) - NX-100162 - Edited the text a little since this dialog now has two lists of codes.
				AfxMessageBox("At least one entry in the adjustment codes to ignore list has no Reason Code selected.\n"
					"All entries must have both a Group Code and a Reason Code selected.");
				return false;
			}

			//make sure this is not a duplicated group
			IRowSettingsPtr pRow2 = m_pIgnoredAdjCodesList->GetFirstRow();
			while (pRow2) {

				//be sure not to check the current row
				if (pRow2 != pRow) {

					// (j.jones 2010-09-23 12:49) - PLID 40653 - these are now IDs
					long nGroupCodeID2 = VarLong(pRow2->GetValue(clcGroupCodeID), -1);
					long nReasonCodeID2 = VarLong(pRow2->GetValue(clcReasonCodeID), -1);

					if (nGroupCodeID == nGroupCodeID2 && nReasonCodeID == nReasonCodeID2) {
						CString str, strGroupCode, strReasonCode;
						_RecordsetPtr rs = CreateParamRecordset("SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}\n "
							"SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}", nGroupCodeID2, nReasonCodeID2);
						if (!rs->eof) {
							strGroupCode = AdoFldString(rs, "Code", "");
						}
						rs = rs->NextRecordset(NULL);
						if (!rs->eof) {
							strReasonCode = AdoFldString(rs, "Code", "");
						}
						rs->Close();
						// (r.gonet 2016-04-18) - NX-100162 - Edited the text a little since this dialog now has two lists of codes.
						str.Format("The following Group Code '%s' and Reason Code '%s' exist more than once in the adjustment codes to ignore list.\n"
							"Each code combination can only be used once. Please remove the duplicate code combination before saving.", strGroupCode, strReasonCode);
						AfxMessageBox(str);
						return false;
					}
				}

				pRow2 = pRow2->GetNextRow();
			}

			pRow = pRow->GetNextRow();
		}
	}

	// (r.gonet 2016-04-18) - NX-100162 - Combined all validation into one function.
	BOOL bWarnedAboutPRCode = FALSE;
	// (r.gonet 2016-04-18) - NX-100162 - Switched the array to a scoped set.
	for(long nChangedID : m_ignoredAdjCodeModifications.setChanged) {
		if (nChangedID != -1) {

			IRowSettingsPtr pRow = m_pIgnoredAdjCodesList->FindByColumn(clcID, (long)nChangedID, m_pIgnoredAdjCodesList->GetFirstRow(), FALSE);
			if (pRow) {
				// (j.jones 2010-09-23 12:49) - PLID 40653 - these are now IDs
				long nGroupCodeID = VarLong(pRow->GetValue(clcGroupCodeID), -1);
				long nReasonCodeID = VarLong(pRow->GetValue(clcReasonCodeID), -1);

				//if the group code is PR, warn the user, but only once
				// (j.jones 2010-09-23 14:32) - PLID 40653 - since these are IDs now, we cached the PR ID
				if (nGroupCodeID == m_nPRGroupCodeID && m_nPRGroupCodeID != -1 && !bWarnedAboutPRCode) {
					// (r.gonet 2016-04-18) - NX-100162 - Edited the text a little because this dialog now has two lists of codes.
					if (MessageBox("For adjustment codes to ignore, at least one group code you have modified is PR - Patient Responsibility.\n"
						"Practice never creates PR adjustments, instead it shifts the given amount to patient responsibility.\n"
						"If you ignore PR codes, Practice will never attempt to shift these amounts to the patient.\n\n"
						"Are you sure you wish to save this change?", "Practice", MB_ICONQUESTION | MB_YESNO) == IDNO) {

						//they said no, so remind them to fix it
						AfxMessageBox("Saving has been cancelled. Please change or remove all lines with the PR group code before saving again.");

						return false;
					}

					//they must have said yes, so don't warn again during this save
					bWarnedAboutPRCode = TRUE;
				}
			} else {
				//don't throw an exception, just assert
				//this shouldn't be possible unless we changed a code,
				//then removed it, and didn't update m_aryChangedCodes
				ASSERT(FALSE);
			}
		}
	}

	//now create new codes
	IRowSettingsPtr pRow = m_pIgnoredAdjCodesList->GetFirstRow();
	while (pRow) {
		long nID = VarLong(pRow->GetValue(clcID), -1);
		if (nID == -1) {
			//new code combination

			// (j.jones 2010-09-23 12:49) - PLID 40653 - these are now IDs
			long nGroupCodeID = VarLong(pRow->GetValue(clcGroupCodeID), -1);
			long nReasonCodeID = VarLong(pRow->GetValue(clcReasonCodeID), -1);

			//if the group code is PR, warn the user, but only once
			// (j.jones 2010-09-23 14:32) - PLID 40653 - since these are IDs now, we cached the PR ID
			if (nGroupCodeID == m_nPRGroupCodeID && m_nPRGroupCodeID != -1 && !bWarnedAboutPRCode) {
				// (r.gonet 2016-04-18) - NX-100162 - Edited the text a litle since this dialog now has two lists of codes.
				if (MessageBox("For adjustment codes to ignore, at least one group code you have created is PR - Patient Responsibility.\n"
					"Practice never creates PR adjustments, instead it shifts the given amount to patient responsibility.\n"
					"If you ignore PR codes, Practice will never attempt to shift these amounts to the patient.\n\n"
					"Are you sure you wish to save this change?", "Practice", MB_ICONQUESTION | MB_YESNO) == IDNO) {

					//they said no, so remind them to fix it
					AfxMessageBox("Saving has been cancelled. Please change or remove all lines with the PR group code before saving again.");
					return false;
				}

				//they must have said yes, so don't warn again during this save
				bWarnedAboutPRCode = TRUE;
			}
		}

		pRow = pRow->GetNextRow();
	}

	return true;
}

// (r.gonet 2016-04-18) - NX-100162 - Encapsulation of the delete ignored adjustment codes logic.
void CAdjustmentCodeSettingsDlg::DeleteRemovedIgnoredAdjCodes(CParamSqlBatch& sqlSaveBatch, CAuditTransaction& auditTransaction)
{
	// (r.gonet 2016-04-18) - NX-100162 - Switched from using an array to a scoped set.
	for(long nDeletedID : m_ignoredAdjCodeModifications.setDeleted) {
		if (nDeletedID != -1) {

			// (b.eyers 2015-03-13) - PLID 42918 - Audit Deletions
			CString strOld, strOldGroupCode, strOldReasonCode;
			_RecordsetPtr rs = CreateParamRecordset("SELECT Code FROM ERemitIgnoredAdjCodesT "
				"LEFT JOIN AdjustmentCodesT ON ERemitIgnoredAdjCodesT.GroupCodeID = AdjustmentCodesT.ID "
				"WHERE ERemitIgnoredAdjCodesT.ID = {INT}\n "
				"SELECT Code FROM ERemitIgnoredAdjCodesT "
				"LEFT JOIN AdjustmentCodesT ON ERemitIgnoredAdjCodesT.ReasonCodeID = AdjustmentCodesT.ID "
				"WHERE ERemitIgnoredAdjCodesT.ID = {INT}", nDeletedID, nDeletedID);
			if (!rs->eof) {
				strOldGroupCode = AdoFldString(rs, "Code", "");
			}
			rs = rs->NextRecordset(NULL);
			if (!rs->eof) {
				strOldReasonCode = AdoFldString(rs, "Code", "");
			}
			rs->Close();
			strOld = strOldGroupCode + " - " + strOldReasonCode;
			// (r.gonet 2016-04-18) - NX-100162 - Changed auditing to use an audit transaction object.
			AuditEvent(-1, "", auditTransaction, aeiERemitAdjCodesToIgnoreDeleted, -1, strOld, "<Deleted>", 2, aetDeleted);

			// (r.gonet 2016-04-18) - NX-100162 - Changed to use a batch object.
			sqlSaveBatch.Add("DELETE FROM ERemitIgnoredAdjCodesT WHERE ID = {INT}", nDeletedID);

		}
	}
}

// (r.gonet 2016-04-18) - NX-100162 - Encapsulation of the update ignored adjustment codes logic.
void CAdjustmentCodeSettingsDlg::UpdateChangedIgnoredAdjCodes(CParamSqlBatch& sqlSaveBatch, CAuditTransaction& auditTransaction)
{
	// (r.gonet 2016-04-18) - NX-100162 - Switched to use a scoped set.
	for (long nChangedID : m_ignoredAdjCodeModifications.setChanged) {
		if (nChangedID != -1) {

			IRowSettingsPtr pRow = m_pIgnoredAdjCodesList->FindByColumn(clcID, (long)nChangedID, m_pIgnoredAdjCodesList->GetFirstRow(), FALSE);
			if (pRow) {
				// (j.jones 2010-09-23 12:49) - PLID 40653 - these are now IDs
				long nGroupCodeID = VarLong(pRow->GetValue(clcGroupCodeID), -1);
				long nReasonCodeID = VarLong(pRow->GetValue(clcReasonCodeID), -1);

				// (r.gonet 2016-04-18) - NX-100162 - Moved validation logic to its own function
				// so it could be done earlier and modularly.
				if (nGroupCodeID == -1 || nReasonCodeID == -1) {
					//should be impossible
					ASSERT(FALSE);
					// (r.gonet 2016-04-18) - NX-100162 - Edit the exception text some since the dialog now has two lists of codes.
					ThrowNxException("%s : Attempted to save invalid group/reason codes!", __FUNCTION__);
				}

				// (b.eyers 2015-03-13) - PLID 42918 - Audit Chanages
				CString strOld, strOldGroupCode, strOldReasonCode, strNew, strNewGroupCode, strNewReasonCode;
				_RecordsetPtr rs = CreateParamRecordset("SELECT Code FROM ERemitIgnoredAdjCodesT "
					"LEFT JOIN AdjustmentCodesT ON ERemitIgnoredAdjCodesT.GroupCodeID = AdjustmentCodesT.ID "
					"WHERE ERemitIgnoredAdjCodesT.ID = {INT}\n "
					"SELECT Code FROM ERemitIgnoredAdjCodesT "
					"LEFT JOIN AdjustmentCodesT ON ERemitIgnoredAdjCodesT.ReasonCodeID = AdjustmentCodesT.ID "
					"WHERE ERemitIgnoredAdjCodesT.ID = {INT}\n "
					"SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}\n "
					"SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}", nChangedID, nChangedID, nGroupCodeID, nReasonCodeID);
				if (!rs->eof) {
					strOldGroupCode = AdoFldString(rs, "Code", "");
				}
				rs = rs->NextRecordset(NULL);
				if (!rs->eof) {
					strOldReasonCode = AdoFldString(rs, "Code", "");
				}
				rs = rs->NextRecordset(NULL);
				if (!rs->eof) {
					strNewGroupCode = AdoFldString(rs, "Code", "");
				}
				rs = rs->NextRecordset(NULL);
				if (!rs->eof) {
					strNewReasonCode = AdoFldString(rs, "Code", "");
				}
				rs->Close();
				strOld = strOldGroupCode + " - " + strOldReasonCode;
				strNew = strNewGroupCode + " - " + strNewReasonCode;
				// (r.gonet 2016-04-18) - NX-100162 - Changed to use an auditing transaction object.
				AuditEvent(-1, "", auditTransaction, aeiERemitAdjCodesToIgnoreModified, -1, strOld, strNew, 2, aetChanged);

				// (r.gonet 2016-04-18) - NX-100162 - Changed to use a sql batch object.
				sqlSaveBatch.Add("UPDATE ERemitIgnoredAdjCodesT "
					"SET GroupCodeID = {INT}, ReasonCodeID = {INT} "
					"WHERE ID = {INT}", nGroupCodeID, nReasonCodeID, nChangedID);

			} else {
				//don't throw an exception, just assert
				//this shouldn't be possible unless we changed a code,
				//then removed it, and didn't update m_aryChangedCodes
				ASSERT(FALSE);
			}
		}
	}
	//do not clear the array, there could be an exception, plus we're closing this dialog anyways
}

// (r.gonet 2016-04-18) - NX-100162 - Encapsulation of the ignored adjustment codes creation logic.
void CAdjustmentCodeSettingsDlg::CreateNewIgnoredAdjCodes(CParamSqlBatch& sqlSaveBatch, CAuditTransaction& auditTransaction)
{
	IRowSettingsPtr pRow = m_pIgnoredAdjCodesList->GetFirstRow();
	while (pRow) {
		long nID = VarLong(pRow->GetValue(clcID), -1);
		if (nID == -1) {
			//new code combination

			// (j.jones 2010-09-23 12:49) - PLID 40653 - these are now IDs
			long nGroupCodeID = VarLong(pRow->GetValue(clcGroupCodeID), -1);
			long nReasonCodeID = VarLong(pRow->GetValue(clcReasonCodeID), -1);

			// (r.gonet 2016-04-18) - NX-100162 - Moved validation logic to a combined validation function
			// so it could be done earlier and modularly.
			if (nGroupCodeID == -1 || nReasonCodeID == 1) {
				//should be impossible
				ASSERT(FALSE);
				// (r.gonet 2016-04-18) - NX-100162 - Edited the exception text slightly because this dialog now has two lists of codes.
				ThrowNxException("%s : Attempted to save invalid group/reason codes!", __FUNCTION__);
			}

			// (b.eyers 2015-03-13) - PLID 42918 - Audit additions
			CString strNew, strNewGroupCode, strNewReasonCode;
			_RecordsetPtr rs = CreateParamRecordset("SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}\n "
				"SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}", nGroupCodeID, nReasonCodeID);
			if (!rs->eof) {
				strNewGroupCode = AdoFldString(rs, "Code", "");
			}
			rs = rs->NextRecordset(NULL);
			if (!rs->eof) {
				strNewReasonCode = AdoFldString(rs, "Code", "");
			}
			rs->Close();
			strNew = strNewGroupCode + " - " + strNewReasonCode;
			// (r.gonet 2016-04-18) - NX-100162 - Changed to use an auditing transaction object.
			AuditEvent(-1, "", auditTransaction, aeiERemitAdjCodesToIgnoreCreated, -1, "", strNew, 2, aetCreated);

			// (r.gonet 2016-04-18) - NX-100162 - Changed to use a sql batch.
			sqlSaveBatch.Add("INSERT INTO ERemitIgnoredAdjCodesT (GroupCodeID, ReasonCodeID) "
				"VALUES ({INT}, {INT})", nGroupCodeID, nReasonCodeID);

		}

		pRow = pRow->GetNextRow();
	}
}

// (r.gonet 2016-04-18) - NX-100162 - Validates that the list of codes allowing negative adjustments to be posted
// is valid. Returns true if validation passed and false if it failed. This function will display the validation failure
// message boxes.
bool CAdjustmentCodeSettingsDlg::ValidateAllowNegativePostingAdjCodes()
{
	//Disallow saving if -1 is selected in either column
	//Disallow saving duplicates
	IRowSettingsPtr pRow = m_pAllowNegativeAdjCodesList->GetFirstRow();
	while (pRow) {
		long nGroupCodeID = VarLong(pRow->GetValue(clcGroupCodeID), -1);
		long nReasonCodeID = VarLong(pRow->GetValue(clcReasonCodeID), -1);

		if (nGroupCodeID == -1) {
			AfxMessageBox("At least one entry in the list of adjustment codes allowing negative adjustments to be posted has no Group Code selected.\n"
				"All entries must have both a Group Code and a Reason Code selected.");
			return false;
		}

		if (nReasonCodeID == -1) {
			AfxMessageBox("At least one entry in the list of adjustment codes allowing negative adjustments to be posted has no Reason Code selected.\n"
				"All entries must have both a Group Code and a Reason Code selected.");
			return false;
		}

		//make sure this is not a duplicated group
		IRowSettingsPtr pRow2 = m_pAllowNegativeAdjCodesList->GetFirstRow();
		while (pRow2) {

			//be sure not to check the current row
			if (pRow2 != pRow) {
				long nGroupCodeID2 = VarLong(pRow2->GetValue(clcGroupCodeID), -1);
				long nReasonCodeID2 = VarLong(pRow2->GetValue(clcReasonCodeID), -1);

				if (nGroupCodeID == nGroupCodeID2 && nReasonCodeID == nReasonCodeID2) {
					CString str, strGroupCode, strReasonCode;
					_RecordsetPtr rs = CreateParamRecordset("SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}\n "
						"SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}", nGroupCodeID2, nReasonCodeID2);
					if (!rs->eof) {
						strGroupCode = AdoFldString(rs, "Code", "");
					}
					rs = rs->NextRecordset(NULL);
					if (!rs->eof) {
						strReasonCode = AdoFldString(rs, "Code", "");
					}
					rs->Close();
					str.Format("The following Group Code '%s' and Reason Code '%s' exist more than once in the list of adjustment codes allowing negative adjustments to be posted.\n"
						"Each code combination can only be used once. Please remove the duplicate code combination before saving.", strGroupCode, strReasonCode);
					AfxMessageBox(str);
					return false;
				}
			}

			pRow2 = pRow2->GetNextRow();
		}

		pRow = pRow->GetNextRow();
	}

	// (r.gonet 2016-04-18) - NX-100162 - Check for the existence of To Ignore adjustment codes that use the same codes. If there are any, that would represent a contradiction
	// if both were to be respected. The reason why we are letting them save at all is because
	// the mod creating the ERemitAllowNegativePostingAdjCodesT table adds as a default CO-144 and product management
	// decided that if that code already exists in the ignored codes list, then this feature just won't work for this
	// particular code. So it wouldn't really make sense to forbid them from saving if it was allowed implicitly before.
	// In fact, if we forbade them from saving in this state but really actually supported this state, then this could
	// lead to bugs in future maintenance programming when it is assumed that you cannot have the same code in both lists.
	bool bWarnedAboutContradiction = false;
	NXDATALIST2Lib::IRowSettingsPtr pAllowNegativeRow = m_pAllowNegativeAdjCodesList->GetFirstRow();
	while (pAllowNegativeRow != nullptr) {
		long nAllowNegativeGroupCodeID = VarLong(pAllowNegativeRow->GetValue(clcGroupCodeID), -1);
		long nAllowNegativeReasonCodeID = VarLong(pAllowNegativeRow->GetValue(clcReasonCodeID), -1);

		NXDATALIST2Lib::IRowSettingsPtr pIgnoreRow = m_pIgnoredAdjCodesList->GetFirstRow();
		while (pIgnoreRow != nullptr) {
			long nIgnoreGroupCodeID = VarLong(pIgnoreRow->GetValue(clcGroupCodeID), -1);
			long nIgnoreReasonCodeID = VarLong(pIgnoreRow->GetValue(clcReasonCodeID), -1);

			if (nAllowNegativeGroupCodeID == nIgnoreGroupCodeID && nAllowNegativeReasonCodeID == nIgnoreReasonCodeID) {
				if (IDYES != MessageBox("There is at least one entry in the list of adjustment codes allowing negative adjustments to be posted that also appears in the list of adjustment codes to ignore. "
					"This will cause Practice to ignore negative adjustments with this group code and reason code when posting the EOB. "
					"Do you want to continue saving?", "Warning", MB_ICONWARNING | MB_YESNO)) 
				{
					return false;
				}
				bWarnedAboutContradiction = true;
				break;
			}

			pIgnoreRow = pIgnoreRow->GetNextRow();
		}

		if (bWarnedAboutContradiction) {
			break;
		}

		pAllowNegativeRow = pAllowNegativeRow->GetNextRow();
	}

	return true;
}

// (r.gonet 2016-04-18) - NX-100162 - Adds SQL statements to a save batch and audits to an audit transaction to save
// the deletions of adjustment codes allowing negative adjustments to be posted.
void CAdjustmentCodeSettingsDlg::DeleteRemovedAllowNegativePostingAdjCodes(CParamSqlBatch& sqlSaveBatch, CAuditTransaction& auditTransaction)
{
	for (long nDeletedID : m_allowNegativePostingAdjCodeModifications.setDeleted) {
		if (nDeletedID != -1) {

			// (b.eyers 2015-03-13) - PLID 42918 - Audit Deletions
			CString strOld, strOldGroupCode, strOldReasonCode;
			_RecordsetPtr rs = CreateParamRecordset("SELECT Code FROM ERemitAllowNegativePostingAdjCodesT "
				"LEFT JOIN AdjustmentCodesT ON ERemitAllowNegativePostingAdjCodesT.GroupCodeID = AdjustmentCodesT.ID "
				"WHERE ERemitAllowNegativePostingAdjCodesT.ID = {INT}\n "
				"SELECT Code FROM ERemitAllowNegativePostingAdjCodesT "
				"LEFT JOIN AdjustmentCodesT ON ERemitAllowNegativePostingAdjCodesT.ReasonCodeID = AdjustmentCodesT.ID "
				"WHERE ERemitAllowNegativePostingAdjCodesT.ID = {INT}", nDeletedID, nDeletedID);
			if (!rs->eof) {
				strOldGroupCode = AdoFldString(rs, "Code", "");
			}
			rs = rs->NextRecordset(NULL);
			if (!rs->eof) {
				strOldReasonCode = AdoFldString(rs, "Code", "");
			}
			rs->Close();
			strOld = strOldGroupCode + " - " + strOldReasonCode;
			AuditEvent(-1, "", auditTransaction, aeiERemitAllowNegativePostingAdjCodeDeleted, -1, strOld, "<Deleted>", 2, aetDeleted);

			sqlSaveBatch.Add("DELETE FROM ERemitAllowNegativePostingAdjCodesT WHERE ID = {INT}", nDeletedID);

		}
	}
}

// (r.gonet 2016-04-18) - NX-100162 - Adds SQL statements to a save batch and audits to an audit transaction to save
// the updates of adjustment codes allowing negative adjustments to be posted.
void CAdjustmentCodeSettingsDlg::UpdateChangedAllowNegativePostingAdjCodes(CParamSqlBatch& sqlSaveBatch, CAuditTransaction& auditTransaction)
{
	for (long nChangedID : m_allowNegativePostingAdjCodeModifications.setChanged) {
		if (nChangedID != -1) {

			IRowSettingsPtr pRow = m_pAllowNegativeAdjCodesList->FindByColumn(clcID, (long)nChangedID, m_pAllowNegativeAdjCodesList->GetFirstRow(), FALSE);
			if (pRow) {
				// (j.jones 2010-09-23 12:49) - PLID 40653 - these are now IDs
				long nGroupCodeID = VarLong(pRow->GetValue(clcGroupCodeID), -1);
				long nReasonCodeID = VarLong(pRow->GetValue(clcReasonCodeID), -1);

				if (nGroupCodeID == -1 || nReasonCodeID == -1) {
					//should be impossible
					ASSERT(FALSE);
					ThrowNxException("%s : Attempted to save invalid group/reason codes!", __FUNCTION__);
				}

				// (b.eyers 2015-03-13) - PLID 42918 - Audit Chanages
				CString strOld, strOldGroupCode, strOldReasonCode, strNew, strNewGroupCode, strNewReasonCode;
				_RecordsetPtr rs = CreateParamRecordset("SELECT Code FROM ERemitAllowNegativePostingAdjCodesT "
					"LEFT JOIN AdjustmentCodesT ON ERemitAllowNegativePostingAdjCodesT.GroupCodeID = AdjustmentCodesT.ID "
					"WHERE ERemitAllowNegativePostingAdjCodesT.ID = {INT}\n "
					"SELECT Code FROM ERemitAllowNegativePostingAdjCodesT "
					"LEFT JOIN AdjustmentCodesT ON ERemitAllowNegativePostingAdjCodesT.ReasonCodeID = AdjustmentCodesT.ID "
					"WHERE ERemitAllowNegativePostingAdjCodesT.ID = {INT}\n "
					"SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}\n "
					"SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}", nChangedID, nChangedID, nGroupCodeID, nReasonCodeID);
				if (!rs->eof) {
					strOldGroupCode = AdoFldString(rs, "Code", "");
				}
				rs = rs->NextRecordset(NULL);
				if (!rs->eof) {
					strOldReasonCode = AdoFldString(rs, "Code", "");
				}
				rs = rs->NextRecordset(NULL);
				if (!rs->eof) {
					strNewGroupCode = AdoFldString(rs, "Code", "");
				}
				rs = rs->NextRecordset(NULL);
				if (!rs->eof) {
					strNewReasonCode = AdoFldString(rs, "Code", "");
				}
				rs->Close();
				strOld = strOldGroupCode + " - " + strOldReasonCode;
				strNew = strNewGroupCode + " - " + strNewReasonCode;
				AuditEvent(-1, "", auditTransaction, aeiERemitAllowNegativePostingAdjCodeModified, -1, strOld, strNew, 2, aetChanged);

				sqlSaveBatch.Add("UPDATE ERemitAllowNegativePostingAdjCodesT "
					"SET GroupCodeID = {INT}, ReasonCodeID = {INT} "
					"WHERE ID = {INT}", nGroupCodeID, nReasonCodeID, nChangedID);

			} else {
				//don't throw an exception, just assert
				//this shouldn't be possible unless we changed a code,
				//then removed it, and didn't update m_aryChangedCodes
				ASSERT(FALSE);
			}
		}
	}
}

// (r.gonet 2016-04-18) - NX-100162 - Adds SQL statements to a save batch and audits to an audit transaction to save
// the creations of adjustment codes allowing negative adjustments to be posted.
void CAdjustmentCodeSettingsDlg::CreateNewAllowNegativePostingAdjCodes(CParamSqlBatch& sqlSaveBatch, CAuditTransaction& auditTransaction)
{
	IRowSettingsPtr pRow = m_pAllowNegativeAdjCodesList->GetFirstRow();
	while (pRow) {
		long nID = VarLong(pRow->GetValue(clcID), -1);
		if (nID == -1) {
			//new code combination

			// (j.jones 2010-09-23 12:49) - PLID 40653 - these are now IDs
			long nGroupCodeID = VarLong(pRow->GetValue(clcGroupCodeID), -1);
			long nReasonCodeID = VarLong(pRow->GetValue(clcReasonCodeID), -1);

			if (nGroupCodeID == -1 || nReasonCodeID == 1) {
				//should be impossible
				ASSERT(FALSE);
				ThrowNxException("%s : Attempted to save invalid group/reason codes!", __FUNCTION__);
			}

			// (b.eyers 2015-03-13) - PLID 42918 - Audit additions
			CString strNew, strNewGroupCode, strNewReasonCode;
			_RecordsetPtr rs = CreateParamRecordset("SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}\n "
				"SELECT Code FROM AdjustmentCodesT WHERE ID = {INT}", nGroupCodeID, nReasonCodeID);
			if (!rs->eof) {
				strNewGroupCode = AdoFldString(rs, "Code", "");
			}
			rs = rs->NextRecordset(NULL);
			if (!rs->eof) {
				strNewReasonCode = AdoFldString(rs, "Code", "");
			}
			rs->Close();
			strNew = strNewGroupCode + " - " + strNewReasonCode;
			AuditEvent(-1, "", auditTransaction, aeiERemitAllowNegativePostingAdjCodeCreated, -1, "", strNew, 2, aetCreated);

			sqlSaveBatch.Add("INSERT INTO ERemitAllowNegativePostingAdjCodesT (GroupCodeID, ReasonCodeID) "
				"VALUES ({INT}, {INT})", nGroupCodeID, nReasonCodeID);

		}

		pRow = pRow->GetNextRow();
	}
}

void CAdjustmentCodeSettingsDlg::OnAddIgnoredAdjCode()
{
	try {

		IRowSettingsPtr pRow = m_pIgnoredAdjCodesList->GetNewRow();
		pRow->PutValue(clcID, (long)-1);
		pRow->PutValue(clcGroupCodeID, (long)-1);
		pRow->PutValue(clcReasonCodeID, (long)-1);
		m_pIgnoredAdjCodesList->AddRowAtEnd(pRow, NULL);
		m_pIgnoredAdjCodesList->PutCurSel(pRow);
		m_pIgnoredAdjCodesList->StartEditing(pRow, clcGroupCodeID);

	}NxCatchAll(__FUNCTION__);
}

void CAdjustmentCodeSettingsDlg::OnRemoveIgnoredAdjCode()
{
	try {

		IRowSettingsPtr pRow = m_pIgnoredAdjCodesList->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must select an entry from the list to delete.");
			return;
		}

		long nID = VarLong(pRow->GetValue(clcID), -1);		

		//is it an existing code?
		if(nID != -1) {

			//add to m_aryDeletedCodes, it should be impossible to already be in the list
			m_ignoredAdjCodeModifications.setDeleted.insert(nID);			
			
			//remove from m_aryChangedCodes, if it exists
			// (r.gonet 2016-04-18) - NX-100162 - Changed from an array to a set.
			m_ignoredAdjCodeModifications.setChanged.erase(nID);
		}

		//remove the row
		m_pIgnoredAdjCodesList->RemoveRow(pRow);

	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CAdjustmentCodeSettingsDlg, CNxDialog)
	ON_EVENT(CAdjustmentCodeSettingsDlg, IDC_EREMIT_CODES_TO_SKIP_LIST, 10, CAdjustmentCodeSettingsDlg::OnEditingFinishedIgnoredAdjCodesList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CAdjustmentCodeSettingsDlg, IDC_ALLOW_NEGATIVE_ADJ_CODES_LIST, 10, CAdjustmentCodeSettingsDlg::EditingFinishedAllowNegativeAdjCodesList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CAdjustmentCodeSettingsDlg, IDC_ALLOW_NEGATIVE_ADJ_CODES_LIST, 28, CAdjustmentCodeSettingsDlg::CurSelWasSetAllowNegativeAdjCodesList, VTS_NONE)
	ON_EVENT(CAdjustmentCodeSettingsDlg, IDC_EREMIT_CODES_TO_SKIP_LIST, 28, CAdjustmentCodeSettingsDlg::CurSelWasSetEremitCodesToSkipList, VTS_NONE)
END_EVENTSINK_MAP()

void CAdjustmentCodeSettingsDlg::OnEditingFinishedIgnoredAdjCodesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		if(!bCommit) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		//if they changed something, add to our changed list
		//(note: we can't tell if they changed something, then
		//changed it back to what it used to be - it's not 
		//really a big deal)
		if((nCol == clcGroupCodeID || nCol == clcReasonCodeID)
			&& VarLong(varOldValue) != VarLong(varNewValue)) {

			long nID = VarLong(pRow->GetValue(clcID), -1);
			if(nID != -1) {
				//they changed an existing code, so add it to our list
				// (r.gonet 2016-04-18) - NX-100162 - Changed from an array to a set.
				m_ignoredAdjCodeModifications.setChanged.insert(nID);
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-04-18) - NX-100162 - Handler for when the user checks/unchecks the
// Allow All Negative Adjustments checkbox. Just enables/disables the datalist below
// the checkbox.
void CAdjustmentCodeSettingsDlg::OnBnClickedAllowAllNegativeAdjustmentsCheck()
{
	try {
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-04-18) - NX-100162 - Handler for when the user clicks the Add button for
// the allow negative adjustments list. Adds a new row to that list.
void CAdjustmentCodeSettingsDlg::OnBnClickedAddAllowNegativeAdjCode()
{
	try {
		IRowSettingsPtr pRow = m_pAllowNegativeAdjCodesList->GetNewRow();
		pRow->PutValue(clcID, (long)-1);
		pRow->PutValue(clcGroupCodeID, (long)-1);
		pRow->PutValue(clcReasonCodeID, (long)-1);
		m_pAllowNegativeAdjCodesList->AddRowAtEnd(pRow, NULL);
		m_pAllowNegativeAdjCodesList->PutCurSel(pRow);
		m_pAllowNegativeAdjCodesList->StartEditing(pRow, clcGroupCodeID);

		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-04-18) - NX-100162 - Handler for when the user clicks the Remote button for
// the allow negative adjustments list. Removes the selected row from that list.
void CAdjustmentCodeSettingsDlg::OnBnClickedRemoveAllowNegativeAdjCode()
{
	try {
		IRowSettingsPtr pRow = m_pAllowNegativeAdjCodesList->GetCurSel();
		if (pRow == NULL) {
			AfxMessageBox("You must select an entry from the list to delete.");
			return;
		}

		long nID = VarLong(pRow->GetValue(clcID), -1);

		//is it an existing code?
		if (nID != -1) {
			m_allowNegativePostingAdjCodeModifications.setDeleted.insert(nID);
			m_ignoredAdjCodeModifications.setChanged.erase(nID);
		}

		//remove the row
		m_pAllowNegativeAdjCodesList->RemoveRow(pRow);

		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-04-18) - NX-100162 - Updates the control enabled states.
void CAdjustmentCodeSettingsDlg::EnsureControls()
{
	if (m_pIgnoredAdjCodesList->CurSel == nullptr) {
		m_btnRemoveIgnoredAdjCode.EnableWindow(FALSE);
	} else {
		m_btnRemoveIgnoredAdjCode.EnableWindow(TRUE);
	}

	BOOL bAllowAllNegativeAdjustments = m_checkAllowAllNegativeAdjustmentsToBePosted.GetCheck() == BST_CHECKED ? TRUE : FALSE;
	m_pAllowNegativeAdjCodesList->Enabled = bAllowAllNegativeAdjustments ? VARIANT_FALSE : VARIANT_TRUE;
	m_pAllowNegativeAdjCodesList->ReadOnly = bAllowAllNegativeAdjustments ? VARIANT_TRUE : VARIANT_FALSE;

	if (m_pAllowNegativeAdjCodesList->CurSel == nullptr) {
		m_btnRemoveAllowNegativeAdjCode.EnableWindow(FALSE);
	} else {
		m_btnRemoveAllowNegativeAdjCode.EnableWindow(bAllowAllNegativeAdjustments ? FALSE : TRUE);
	}
	m_btnAddAllowNegativeAdjCode.EnableWindow(bAllowAllNegativeAdjustments ? FALSE : TRUE);
}

// (r.gonet 2016-04-18) - NX-100162 - Handles when the user changes one of the embedded code dropdowns
// in the allow negative adjustments datalist.
void CAdjustmentCodeSettingsDlg::EditingFinishedAllowNegativeAdjCodesList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		if (!bCommit) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if (pRow == nullptr) {
			return;
		}

		//if they changed something, add to our changed list
		//(note: we can't tell if they changed something, then
		//changed it back to what it used to be - it's not 
		//really a big deal)
		if ((nCol == clcGroupCodeID || nCol == clcReasonCodeID)
			&& VarLong(varOldValue) != VarLong(varNewValue)) {

			long nID = VarLong(pRow->GetValue(clcID), -1);
			if (nID != -1) {
				m_allowNegativePostingAdjCodeModifications.setChanged.insert(nID);
			} else {
				// It was an unsaved entry.
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-04-18) - NX-100162 - If something changes the current selection, we 
// want to make sure the Remove button is enabled or disabled appropriately.
void CAdjustmentCodeSettingsDlg::CurSelWasSetAllowNegativeAdjCodesList()
{
	try {
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-04-18) - NX-100162 - If something changes the current selection, we 
// want to make sure the Remove button is enabled or disabled appropriately. Added to match
// the functionality of the allow negative adjustments list.
void CAdjustmentCodeSettingsDlg::CurSelWasSetEremitCodesToSkipList()
{
	try {
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}
