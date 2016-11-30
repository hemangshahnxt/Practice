// HL7GeneralSettingsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialRc.h"
#include "HL7GeneralSettingsDlg.h"
#include "HL7Utils.h"
#include <NxHL7Lib/HL7Logging.h>

//TES 9/15/2011 - PLID 45523 - Created
// CHL7GeneralSettingsDlg dialog

IMPLEMENT_DYNAMIC(CHL7GeneralSettingsDlg, CNxDialog)

CHL7GeneralSettingsDlg::CHL7GeneralSettingsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7GeneralSettingsDlg::IDD, pParent)
{

}

CHL7GeneralSettingsDlg::~CHL7GeneralSettingsDlg()
{
}

void CHL7GeneralSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	DDX_Control(pDX, IDC_SENDING_FACILITY, m_nxeSendingFacility);
	DDX_Control(pDX, IDC_FORCE_FACILITY_ID_MATCH, m_nxbForceFacilityIDMatch);
	DDX_Control(pDX, IDC_TRIM_SECONDS, m_nxbTrimSeconds);
	DDX_Control(pDX, IDC_PREPEND_TIMESTAMP, m_nxbPrependTimestamp);
	DDX_Control(pDX, IDC_RECEIVING_APPLICATION, m_nxeReceivingApplication);
	DDX_Control(pDX, IDC_RECEIVING_FACILITY, m_nxeReceivingFacility);
	DDX_Control(pDX, IDC_SENDING_APPLICATION, m_nxeSendingApplication);
	DDX_Control(pDX, IDC_FACILITY_CODE_OVERRIDE_LABEL, m_nxsOverrideFacilityCodeLabel);
	DDX_Control(pDX, IDC_ADD_OVERRIDE_FACILITY_CODE_BTN, m_nxbAddOverride);
	DDX_Control(pDX, IDC_REMOVE_OVERRIDE_FACILITY_CODE_BTN, m_nxbRemoveOverride);
	DDX_Control(pDX, IDC_EDIT_VERSION_OVERRIDE, m_nxeVersionOverride);
}


BEGIN_MESSAGE_MAP(CHL7GeneralSettingsDlg, CNxDialog)
	ON_EN_CHANGE(IDC_SENDING_FACILITY, &CHL7GeneralSettingsDlg::OnEnChangeSendingFacility)
	ON_BN_CLICKED(IDC_ADD_OVERRIDE_FACILITY_CODE_BTN, &CHL7GeneralSettingsDlg::OnBnClickedAddOverrideFacilityCodeBtn)
	ON_BN_CLICKED(IDC_REMOVE_OVERRIDE_FACILITY_CODE_BTN, &CHL7GeneralSettingsDlg::OnBnClickedRemoveOverrideFacilityCodeBtn)
	ON_BN_CLICKED(IDC_FORCE_FACILITY_ID_MATCH, &CHL7GeneralSettingsDlg::OnBnClickedForceFacilityIdMatch)
END_MESSAGE_MAP()


// CHL7GeneralSettingsDlg message handlers
BOOL CHL7GeneralSettingsDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		// (r.gonet 05/01/2014) - PLID 61842 - Added a combo box to select how much logging to do for this link.
		m_pLogLevelCombo = BindNxDataList2Ctrl(IDC_LOG_LEVEL_COMBO, GetRemoteData(), false);
		m_pLogLevelCombo->FromClause = _bstr_t(FormatString(
			"( "
				"SELECT %li AS ID, 'Normal Logging' AS Level "
				"UNION "
				"SELECT %li AS ID, 'Diagnostic Logging' "
			") SubQ ",
			elelError | elelWarning | elelInfo,
			elelError | elelWarning | elelInfo | elelDebug));
		m_pLogLevelCombo->Requery();
		m_pLogLevelCombo->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		DWORD dwCurrentLevel = (DWORD)GetHL7SettingInt(m_nHL7GroupID, "CurrentLoggingLevel");
		m_pLogLevelCombo->FindByColumn(ellcID, _variant_t((long)dwCurrentLevel, VT_I4), NULL, VARIANT_TRUE);

		// (r.gonet 10/31/2011) - PLID 45367 - Bind the provider specific facility codes list
		m_pOverrideFacilityCodeList = BindNxDataList2Ctrl(IDC_FACILITY_CODE_OVERRIDE_LIST, GetRemoteData(), false);
		m_pOverrideFacilityCodeList->WhereClause = _bstr_t(FormatString("HL7GroupID = %li ", m_nHL7GroupID));
		m_pOverrideFacilityCodeList->Requery();
		m_pOverrideFacilityCodeList->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);

		// (r.gonet 10/31/2011) - PLID 45367
		m_nxbAddOverride.AutoSet(NXB_NEW);
		m_nxbRemoveOverride.AutoSet(NXB_DELETE);

		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		//TES 9/15/2011 - PLID 45523 - Moved LabFacilityID and ForceFacilityIDMatch from CHL7SettingsDlg
		m_nxeSendingFacility.SetLimitText(227);

		CString strSendingFacilityCode = GetHL7SettingText(m_nHL7GroupID, "LabFacilityID");
		SetDlgItemText(IDC_SENDING_FACILITY, strSendingFacilityCode);
		ReflectSendingFacility();

		m_nxbForceFacilityIDMatch.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "ForceFacilityIDMatch")?BST_CHECKED:BST_UNCHECKED);

		//TES 9/16/2011 - PLID 45520 - Added TrimeTimestampSeconds
		m_nxbTrimSeconds.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "TrimTimestampSeconds")?BST_CHECKED:BST_UNCHECKED);

		// (r.farnworth 2014-12-19 09:11) - PLID 64469 - Create a preference in HL7 Advanced settings to prepend a timestamp to outgoing HL7 messages
		m_nxbPrependTimestamp.SetCheck(GetHL7SettingBit(m_nHL7GroupID, "PrependTimestampToFilename") ? BST_CHECKED : BST_UNCHECKED);

		//TES 9/16/2011 - PLID 45522 - Added SendingApplication, ReceivingApplication, and ReceivingFacility
		//TES 10/30/2011 - PLID 45522 - Yes, somebody tested putting > 3000 characters in these fields.
		m_nxeSendingApplication.SetLimitText(3000);
		m_nxeReceivingApplication.SetLimitText(3000);
		m_nxeReceivingFacility.SetLimitText(3000);
		SetDlgItemText(IDC_SENDING_APPLICATION, GetHL7SettingText(m_nHL7GroupID, "SendingApplication"));
		SetDlgItemText(IDC_RECEIVING_APPLICATION, GetHL7SettingText(m_nHL7GroupID, "ReceivingApplication"));
		SetDlgItemText(IDC_RECEIVING_FACILITY, GetHL7SettingText(m_nHL7GroupID, "ReceivingFacility"));

		// (b.savon 2014-12-09 14:36) - PLID 64318 - Add a new text box to the HL7 configuration that will be used to override the version of HL7 messages.
		m_nxeVersionOverride.SetLimitText(255);
		CString strVersionOverride = GetHL7SettingText(m_nHL7GroupID, "GlobalVersionOverride");
		if (strVersionOverride.CompareNoCase("2.5.1") != 0){
			m_nxeVersionOverride.SetWindowText(strVersionOverride);
		}

		// (r.gonet 11/06/2011) - PLID 45367 - Set all controls to their correct states.
		EnsureControls();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CHL7GeneralSettingsDlg::ReflectSendingFacility()
{
	// (r.gonet 11/06/2011) - PLID 45367 - We don't currently support forcing facility id to match when we have overrides.
	if(m_pOverrideFacilityCodeList->GetRowCount() > 0) {
		m_nxbForceFacilityIDMatch.EnableWindow(FALSE);
		return;
	} else {
		// We're fine enabling this if we have no overrides.
	}

	//TES 3/10/2011 - PLID 41912 - Enable/Disable m_btnForceFacilityIDMatch based on whether the facility ID is empty; if the facility ID
	// is empty then we're never going to compare against it anyway.
	CString strSendingFacility;
	GetDlgItemText(IDC_SENDING_FACILITY, strSendingFacility);
	m_nxbForceFacilityIDMatch.EnableWindow(!strSendingFacility.IsEmpty());
}

void CHL7GeneralSettingsDlg::OnOK()
{
	try {
		// (r.gonet 05/01/2014) - PLID 61842 - Save the desired logging level.
		NXDATALIST2Lib::IRowSettingsPtr pLoggingLevelRow = m_pLogLevelCombo->CurSel;
		if (pLoggingLevelRow != NULL) {
			DWORD dwDefaultLoggingLevel = elelError | elelWarning | elelInfo;
			DWORD dwLoggingLevel = (DWORD)VarLong(pLoggingLevelRow->GetValue(ellcID), (long)dwDefaultLoggingLevel);
			SetHL7SettingInt(m_nHL7GroupID, "CurrentLoggingLevel", (long)dwLoggingLevel);
		}

		// (r.gonet 11/06/2011) - PLID 45367 - Validate the overrides. We override per provider, so obviously need a provider.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOverrideFacilityCodeList->GetFirstRow();
		while(pRow) {
			// (r.gonet 01/15/2013) - PLID 54287 - Get the location as well.
			long nLocationID = VarLong(pRow->GetValue(eofccLocationID), -1);
			long nProviderID = VarLong(pRow->GetValue(eofccProviderID), -1);
			// (r.gonet 01/15/2013) - PLID 54287 - Only if the override has neither a location nor a provider do we fail.
			if(nLocationID == -1 && nProviderID == -1) {
				MessageBox("All facility code overrides must have a location, a provider, or both associated with them.", "Error", MB_OK|MB_ICONERROR);
				return;
			} else {
				// User has selected a provider or location. We require this of each override.
			}
			pRow = pRow->GetNextRow();
		}

		// (r.gonet 11/06/2011) - PLID 45367 - Validate the overrides. Providers must have at most one override otherwise things get ambiguous as to which to use.
		// (r.gonet 01/15/2013) - PLID 54287 - The user may not know about the priority of the location over the provider, so warn if we have such a case.
		bool bWarnAboutPrecedence = false;
		pRow = m_pOverrideFacilityCodeList->GetFirstRow();
		while(pRow) {
			long nLocationID1 = VarLong(pRow->GetValue(eofccLocationID), -1);
			// (r.gonet 01/15/2013) - PLID 54287 - Get the location as well.
			long nProviderID1 = VarLong(pRow->GetValue(eofccProviderID), -1);
			NXDATALIST2Lib::IRowSettingsPtr pRow2 = m_pOverrideFacilityCodeList->GetFirstRow();
			while(pRow2) {
				long nLocationID2 = VarLong(pRow2->GetValue(eofccLocationID), -1);
				// (r.gonet 01/15/2013) - PLID 54287 - Get the location as well.
				long nProviderID2 = VarLong(pRow2->GetValue(eofccProviderID), -1);
				if(pRow != pRow2 && nLocationID1 == nLocationID2 && nProviderID1 == nProviderID2) {
					MessageBox("All facility code overrides must have a unique location and provider.", "Error", MB_OK|MB_ICONERROR);
					return;
				} else if(pRow != pRow2 && nLocationID1 != -1 && nProviderID2 != -1) {
					// (r.gonet 01/15/2013) - PLID 54287 - So one row has a Location only. Another row has a Provider only. 
					//  The Location only override will take priority over the Provider only override. Warn the user about this.
					bWarnAboutPrecedence = true;
				} else {
					// Locations or Providers differ, and that is what we require of each.
				}
				pRow2 = pRow2->GetNextRow();
			}
			pRow = pRow->GetNextRow();
		}

		// (r.gonet 01/15/2013) - PLID 54287
		if(bWarnAboutPrecedence) {
			CString strPropName = "FacilityCodeOverridesPriority_" + FormatString("%li", m_nHL7GroupID);
			DontShowMeAgain(this, 
				"At least two overrides conflict in their matching criteria. In other words, they can both be true at the same time. "
				"This is okay, but be aware that a more specific override will take priority over a more general override, and also that "
				"a location-only override will take priority over a provider-only override.",
				strPropName);
		}

		//TES 9/15/2011 - PLID 45523 - Moved LabFacilityID and ForceFacilityIDMatch from CHL7SettingsDlg
		CString strSendingFacility;
		GetDlgItemText(IDC_SENDING_FACILITY, strSendingFacility);
		SetHL7SettingText(m_nHL7GroupID, "LabFacilityID", strSendingFacility);
		
		SetHL7SettingBit(m_nHL7GroupID, "ForceFacilityIDMatch", IsDlgButtonChecked(IDC_FORCE_FACILITY_ID_MATCH));

		//TES 9/16/2011 - PLID 45520 - Added TrimeTimestampSeconds
		SetHL7SettingBit(m_nHL7GroupID, "TrimTimestampSeconds", IsDlgButtonChecked(IDC_TRIM_SECONDS));

		// (r.farnworth 2014-12-19 09:09) - PLID 64469 - Create a preference in HL7 Advanced settings to prepend a timestamp to outgoing HL7 messages
		SetHL7SettingBit(m_nHL7GroupID, "PrependTimestampToFilename", IsDlgButtonChecked(IDC_PREPEND_TIMESTAMP));

		//TES 9/16/2011 - PLID 45522 - Added SendingApplication, ReceivingApplication, and ReceivingFacility
		CString strSendingApplication;
		GetDlgItemText(IDC_SENDING_APPLICATION, strSendingApplication);
		SetHL7SettingText(m_nHL7GroupID, "SendingApplication", strSendingApplication);
		CString strReceivingApplication;
		GetDlgItemText(IDC_RECEIVING_APPLICATION, strReceivingApplication);
		SetHL7SettingText(m_nHL7GroupID, "ReceivingApplication", strReceivingApplication);
		CString strReceivingFacility;
		GetDlgItemText(IDC_RECEIVING_FACILITY, strReceivingFacility);
		SetHL7SettingText(m_nHL7GroupID, "ReceivingFacility", strReceivingFacility);

		// (r.gonet 10/31/2011) - PLID 45367 - Delete any override facility codes that the user has removed
		CParamSqlBatch sqlBatch;
		if(m_aryDeletedOverrideFacilityCodes.GetSize() > 0) {
			sqlBatch.Add("DELETE FROM HL7OverrideFacilityCodesT WHERE ID IN ({INTARRAY})", m_aryDeletedOverrideFacilityCodes);
		} else {
			// Nothing to delete
		}
		
		// (r.gonet 10/31/2011) - PLID 45367 - Now, save any updated or new override codes.
		pRow = m_pOverrideFacilityCodeList->GetFirstRow();
		while(pRow) {
			long nID = VarLong(pRow->GetValue(eofccID), -1);
			// (r.gonet 01/15/2013) - PLID 54287 - Get the location as well.
			long nLocationID = VarLong(pRow->GetValue(eofccLocationID), -1);
			_variant_t varLocationID = (nLocationID == -1 ? g_cvarNull : _variant_t(nLocationID, VT_I4));
			long nProviderID = VarLong(pRow->GetValue(eofccProviderID), -1);
			_variant_t varProviderID = (nProviderID == -1 ? g_cvarNull : _variant_t(nProviderID, VT_I4));
			CString strSendFacilityCode = VarString(pRow->GetValue(eofccSendingFacilityCode), "");
			CString strRecvFacilityCode = VarString(pRow->GetValue(eofccReceivingFacilityCode), "");
			BOOL bModified = VarBool(pRow->GetValue(eofccModified), FALSE);
			if(nID == -1) {
				// (r.gonet 01/15/2013) - PLID 54287 - Added the location.
				sqlBatch.Add(
					"INSERT INTO HL7OverrideFacilityCodesT (HL7GroupID, LocationID, ProviderID, SendingFacilityCode, ReceivingFacilityCode) "
					"VALUES "
					"({INT}, {VT_I4}, {VT_I4}, {STRING}, {STRING}); ",
					m_nHL7GroupID, varLocationID, varProviderID, strSendFacilityCode, strRecvFacilityCode);
			} else if(bModified) {
				// (r.gonet 01/15/2013) - PLID 54287 - Added the location.
				sqlBatch.Add(
					"UPDATE HL7OverrideFacilityCodesT SET "
					"	LocationID = {VT_I4}, "
					"	ProviderID = {VT_I4}, "
					"	SendingFacilityCode = {STRING}, "
					"	ReceivingFacilityCode = {STRING} "
					"WHERE ID = {INT}",
					varLocationID, varProviderID, strSendFacilityCode, strRecvFacilityCode,
					nID);
			} else {
				// Override is unchanged
			}
			pRow = pRow->GetNextRow();
		}
		sqlBatch.Execute(GetRemoteData());

		// (b.savon 2014-12-09 14:52) - PLID 64318 - Add a new text box to the HL7 configuration that will be used to override the version of HL7 messages. If nothing is in this text box, 2.5.1 will be the version.
		CString strVersionOverride;
		m_nxeVersionOverride.GetWindowText(strVersionOverride);
		if (strVersionOverride.IsEmpty()){
			SetHL7SettingText(m_nHL7GroupID, "GlobalVersionOverride", "2.5.1");
		}
		else{
			SetHL7SettingText(m_nHL7GroupID, "GlobalVersionOverride", strVersionOverride);
		}
		

		CClient::RefreshTable(NetUtils::HL7SettingsT, m_nHL7GroupID);
		RefreshHL7Group(m_nHL7GroupID);

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}
void CHL7GeneralSettingsDlg::OnEnChangeSendingFacility()
{
	try {
		//TES 9/15/2011 - PLID 45523 - Disable the ForceFacilityIDMatch box if the ID is empty
		ReflectSendingFacility();
	}NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/31/2011) - PLID 45367 - Add a new override code to the list and commit to insert it when we save.
void CHL7GeneralSettingsDlg::OnBnClickedAddOverrideFacilityCodeBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pOverrideFacilityCodeList->GetNewRow();
		pNewRow = m_pOverrideFacilityCodeList->GetNewRow();
		pNewRow->PutValue(eofccID, _variant_t(-1L, VT_I4));
		// (r.gonet 01/15/2013) - PLID 54287 - Fill the location as well.
		pNewRow->PutValue(eofccLocationID, _variant_t(-1L, VT_I4));
		pNewRow->PutValue(eofccProviderID, _variant_t(-1L, VT_I4));
		pNewRow->PutValue(eofccReceivingFacilityCode, _variant_t(""));
		pNewRow->PutValue(eofccSendingFacilityCode, _variant_t(""));
		pNewRow->PutValue(eofccModified, g_cvarTrue);
		m_pOverrideFacilityCodeList->AddRowAtEnd(pNewRow, NULL);
		m_pOverrideFacilityCodeList->CurSel = pNewRow;
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/31/2011) - PLID 45367 - Remove the currently selected override code from the list and commit to delete it when we save.
void CHL7GeneralSettingsDlg::OnBnClickedRemoveOverrideFacilityCodeBtn()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOverrideFacilityCodeList->CurSel;
		if(pRow) {
			long nID = VarLong(pRow->GetValue(eofccID), -1);
			if(nID != -1) {
				// This facility code override has already been saved to the database, so we'll need to delete it from the db upon saving. Remember it.
				m_aryDeletedOverrideFacilityCodes.Add(nID);
			} else {
				// We don't need to delete anything from the DB. Just don't save the override at all.
			}

			m_pOverrideFacilityCodeList->RemoveRow(pRow);
			EnsureControls();
		} else {
			// How was this clicked if there is no row? EnsureControls should have handled the disabling and prevented this branch.
		}
	} NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CHL7GeneralSettingsDlg, CNxDialog)
	ON_EVENT(CHL7GeneralSettingsDlg, IDC_FACILITY_CODE_OVERRIDE_LIST, 2, CHL7GeneralSettingsDlg::SelChangedFacilityCodeOverrideList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CHL7GeneralSettingsDlg, IDC_FACILITY_CODE_OVERRIDE_LIST, 9, CHL7GeneralSettingsDlg::EditingFinishingFacilityCodeOverrideList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CHL7GeneralSettingsDlg, IDC_FACILITY_CODE_OVERRIDE_LIST, 10, CHL7GeneralSettingsDlg::EditingFinishedFacilityCodeOverrideList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CHL7GeneralSettingsDlg, IDC_LOG_LEVEL_COMBO, 1, CHL7GeneralSettingsDlg::SelChangingLogLevelCombo, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

// (r.gonet 10/31/2011) - PLID 45367 - The user changed their selection, update the buttons
void CHL7GeneralSettingsDlg::SelChangedFacilityCodeOverrideList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);
		EnsureControls();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/31/2011) - PLID 45367 - Put the controls in the right state.
void CHL7GeneralSettingsDlg::EnsureControls()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOverrideFacilityCodeList->CurSel;
	// Remove is allowed only when we have a row selected
	m_nxbRemoveOverride.EnableWindow(pRow != NULL);

	// We don't currently save the facility ID that was associated with a sent message. If we didn't disallow this here,
	//  we would have to parse out the provider ID and names, look to see if it was associated with a provider in the system,
	//  if not, we would need to prompt the user to associate the provider. Then do a SQL lookup to see if there was an override
	//  for that provider. If so, compare the facility ID that we are getting back with the one that we should have gotten back.
	//  Too much work, may require user input, and adds additional queries at runtime. Way easier to just disable this feature if there
	//  are overrides.
	if(m_pOverrideFacilityCodeList->GetRowCount() > 0 && m_nxbForceFacilityIDMatch.GetCheck()) {
		MessageBox("Force facility ID to match is not supported while there are overrides.", "Error", MB_OK|MB_ICONERROR);
		m_nxbForceFacilityIDMatch.SetCheck(FALSE);
	} else {
		// The user doesn't have selected facility code match. So don't change anything.
	}

	ReflectSendingFacility();
}

// (r.gonet 01/15/2013) - PLID 54287 - Don't let the user choose bad values
void CHL7GeneralSettingsDlg::EditingFinishingFacilityCodeOverrideList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			switch(nCol) {
				case eofccLocationID:
				case eofccProviderID:
					if(pvarNewValue->vt != VT_I4) {
						*pvarNewValue = varOldValue;
						*pbCommit = FALSE;
						return;
					}
					break;
				default:
					break;
			}
		} else {
			// This shouldn't be possible. What would we have finished editing then?
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/31/2011) - PLID 45367 - If the user modified any value, then we will save it later.
void CHL7GeneralSettingsDlg::EditingFinishedFacilityCodeOverrideList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow) {
			switch(nCol) {
				case eofccLocationID:
					// (r.gonet 01/15/2013) - PLID 54287 - LocationID follows the same logic.
				case eofccProviderID:
					// The user changed the location or provider associated with this override. 
					if(VarLong(varOldValue, -1) != VarLong(varNewValue, -1)) {
						pRow->PutValue(eofccModified, g_cvarTrue);
					} else {
						// The value doesn't differ from what it previously was, so don't bother updating it.
					}
					break;
				case eofccSendingFacilityCode:
					// The user changed the sending facility code associated with this override.
					if(varOldValue.vt == VT_BSTR && varNewValue.vt == VT_BSTR && VarString(varOldValue) != VarString(varNewValue)) {
						pRow->PutValue(eofccModified, g_cvarTrue);
					} else {
						// Either of the following is then true: One of the newly selected values is not an int. 
						//  Then what is it? Can't be null since "" takes care of that. Anyway, its invalid so don't save it.
						//  Or more likely, the value just doesn't differ from what it previously was, so don't bother updating it.
					}
					break;
				case eofccReceivingFacilityCode:
					// The user changed the receiving facility code associated with this override.
					if(varOldValue.vt == VT_BSTR && varNewValue.vt == VT_BSTR && VarString(varOldValue) != VarString(varNewValue)) {
						pRow->PutValue(eofccModified, g_cvarTrue);
					} else {
						// Either of the following is then true: One of the newly selected values is not a string. 
						//  Then what is it? Can't be null since "" takes care of that. Anyway, its invalid so don't save it.
						//  Or more likely, the value just doesn't differ from what it previously was, so don't bother updating it.
					}
					break;
				default:
					ASSERT(FALSE); // Nothing else can be edited at this time.
					break;
			}
		} else {
			// This shouldn't be possible. What would we have finished editing then?
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 10/31/2011) - PLID 45367 - Disallow forcing when we have overrides
void CHL7GeneralSettingsDlg::OnBnClickedForceFacilityIdMatch()
{
	try {
		if(m_pOverrideFacilityCodeList->GetRowCount() > 0 && m_nxbForceFacilityIDMatch.GetCheck()) {
			// I don't think this branch can ever be taken since EnsureControls will disable the checkbox. But here's an in case.
			MessageBox("Force facility ID to match is not supported while there are overrides.", "Error", MB_OK|MB_ICONERROR);
			m_nxbForceFacilityIDMatch.SetCheck(FALSE);
		} else {
			// No overrides, no problem enabling this feature then.
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 05/01/2014) - PLID 61842 - Don't let the user select nothing
void CHL7GeneralSettingsDlg::SelChangingLogLevelCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	} NxCatchAll(__FUNCTION__);
}