// NewCropSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "NewCropSetupDlg.h"
#include "Patientsrc.h"
#include "NewCropUtils.h"
#include "NewCropNameOverridesDlg.h"

// CNewCropSetupDlg dialog

// (j.jones 2009-02-27 12:03) - PLID 33265 - created

using namespace NXDATALIST2Lib;

// (j.jones 2011-06-17 09:57) - PLID 41709 - added provider role list
enum ProviderRoleListColumns {
	prPersonID = 0,
	prName,
	prRoleTypeID,
};


enum UserTypeListColumns {
	utlcPersonID = 0,
	utlcUserName,
	utlcUserTypeID,
	utlcLicensedPrescriberID,	// (j.jones 2009-06-08 09:34) - PLID 34514
	utlcMidLevelProviderID,	// (j.jones 2011-06-17 09:53) - PLID 41709
	utlcSupervisingProviderID, // (j.jones 2009-08-24 17:41) - PLID 35203
};

CNewCropSetupDlg::CNewCropSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNewCropSetupDlg::IDD, pParent)
{
	//m_bWasProduction = FALSE;
}

CNewCropSetupDlg::~CNewCropSetupDlg()
{
}

void CNewCropSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_RADIO_NEWCROP_PREPRODUCTION, m_radioPreProduction);
	DDX_Control(pDX, IDC_RADIO_NEWCROP_PRODUCTION, m_radioProduction);
	DDX_Control(pDX, IDC_CHECK_SUPPRESS_PAT_GENDER, m_checkSuppressPatGender);
	DDX_Control(pDX, IDC_BTN_NEWCROP_OVERRIDE_NAMES, m_btnSetupNameOverrides);
}

BEGIN_MESSAGE_MAP(CNewCropSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDC_BTN_NEWCROP_OVERRIDE_NAMES, OnBtnNewcropOverrideNames)
END_MESSAGE_MAP()

BOOL CNewCropSetupDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		// (j.jones 2011-06-17 08:35) - PLID 44157 - moved the name overrides to a new dialog
		m_btnSetupNameOverrides.AutoSet(NXB_MODIFY);

		// (j.jones 2010-04-13 16:09) - PLID 38183 - added NewCropHideProviderSuffix
		g_propManager.CachePropertiesInBulk("CNewCropSetupDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'NewCropAccountLocationID' OR "
			// (j.jones 2010-06-09 11:04) - PLID 39013 - this ConfigRT setting has been renamed
			//"Name = 'NewCropUseProductionServer' "
			"Name = 'NewCrop_ProductionStatusOverride' OR "
			// (j.jones 2011-03-07 11:46) - PLID 42313 - added an option to not send patient gender
			"Name = 'NewCrop_DoNotSendPatientGender' "
			")",
			_Q(GetCurrentUserName()));

		m_LocationCombo = BindNxDataList2Ctrl(IDC_NEWCROP_LOCATION_ACCOUNT_COMBO, true);

		// (j.jones 2011-06-17 09:49) - PLID 41709 - added provider role datalist
		m_pProviderRoleList = BindNxDataList2Ctrl(IDC_NEWCROP_SETUP_PROVIDER_ROLE_LIST, false);

		IColumnSettingsPtr pProviderRoleCol;
		pProviderRoleCol = m_pProviderRoleList->GetColumn(prRoleTypeID);
		CString strComboSource;
		strComboSource.Format("SELECT %li as ID, '<None>' AS ProviderRole "
			"UNION SELECT %li, 'Licensed Prescriber' "
			"UNION SELECT %li, 'Midlevel Provider' ",
			ncprNone, ncprLicensedPrescriber, ncprMidlevelProvider);
		pProviderRoleCol->PutComboSource(_bstr_t(strComboSource));

		m_pProviderRoleList->Requery();

		// (j.gruber 2009-03-31 09:27) - PLID 33328 - added usertypelist
		m_pUserTypeList = BindNxDataList2Ctrl(IDC_NEWCROP_SETUP_USER_LIST, false);

		// (j.gruber 2009-06-08 10:43) - PLID 34515 - add staff/nurse to the selection
		// (j.jones 2009-08-18 16:23) - PLID 35203 - supported Midlevel Prescriber
		IColumnSettingsPtr pUserTypeCol;
		pUserTypeCol = m_pUserTypeList->GetColumn(utlcUserTypeID);
		strComboSource.Format("SELECT %li as ID, '<None>' as UserType "
			"UNION SELECT %li, 'Licensed Prescriber' "
			"UNION SELECT %li, 'Nurse/Staff' "
			"UNION SELECT %li, 'Midlevel Provider' ",
			ncutNone, ncutLicensedPrescriber, ncutStaff_Nurse,
			ncutMidlevelProvider);
		pUserTypeCol->PutComboSource(_bstr_t(strComboSource));

		// (j.jones 2009-08-25 09:46) - PLID 35203 - for the Linked Provider and
		// Supervising Provider columns, we want them to sometimes show the -1 row,
		// and sometimes be empty, when the data is NULL
		IColumnSettingsPtr pLinkedProvCol, pMidlevelProvCol, pSupervisingProvCol;
		pLinkedProvCol = m_pUserTypeList->GetColumn(utlcLicensedPrescriberID);
		// (j.jones 2011-06-17 09:53) - PLID 41709 - added midlevel column
		pMidlevelProvCol = m_pUserTypeList->GetColumn(utlcMidLevelProviderID);
		pSupervisingProvCol = m_pUserTypeList->GetColumn(utlcSupervisingProviderID);

		CString strLinkedProvSource, strMidlevelProvSource, strSupervisingProvSource;
		// (j.jones 2011-06-17 09:53) - PLID 41709 - now licensed & midlevel columns
		// are only editable for licensed & midlevel roles
		strLinkedProvSource.Format("CASE WHEN NewCropUserTypeID = %li "
			"THEN Coalesce(NewCropProviderID, -1) ELSE NewCropProviderID END",
			ncutLicensedPrescriber);
		strMidlevelProvSource.Format("CASE WHEN NewCropUserTypeID = %li "
			"THEN Coalesce(NewCropMidlevelProviderID, -1) ELSE NewCropMidlevelProviderID END",
			ncutMidlevelProvider);
		strSupervisingProvSource.Format("CASE WHEN NewCropUserTypeID = %li OR NewCropUserTypeID = %li "
			"THEN Coalesce(NewCropSupervisingProviderID, -1) ELSE NewCropSupervisingProviderID END",
			ncutStaff_Nurse, ncutMidlevelProvider);

		pLinkedProvCol->PutFieldName(_bstr_t(strLinkedProvSource));
		pMidlevelProvCol->PutFieldName(_bstr_t(strMidlevelProvSource));
		pSupervisingProvCol->PutFieldName(_bstr_t(strSupervisingProvSource));

		//set the sort		
		IColumnSettingsPtr pUserNameCol = m_pUserTypeList->GetColumn(utlcUserName);

		if (pUserNameCol) {
			pUserNameCol->SortPriority = 1;
		}

		//set the where clause
		m_pUserTypeList->WhereClause = "PersonT.Archived = 0 AND PersonT.ID > 0";

		//now requery
		m_pUserTypeList->Requery();

		// (j.jones 2011-03-07 11:46) - PLID 42313 - added an option to not send patient gender
		m_checkSuppressPatGender.SetCheck(GetRemotePropertyInt("NewCrop_DoNotSendPatientGender", 0, 0, "<None>", true) == 1);

		//this setting will default to whatever location you're currently using
		long nLocationID = GetRemotePropertyInt("NewCropAccountLocationID", GetCurrentLocationID(), 0, "<None>", true);
		m_LocationCombo->SetSelByColumn(0, nLocationID);

		// (j.jones 2010-06-09 11:04) - PLID 39013 - production status is read-only now, and is calculated by GetNewCropIsProduction()
		if(GetNewCropIsProduction()) {
			m_radioProduction.SetCheck(TRUE);
		}
		else {
			m_radioPreProduction.SetCheck(TRUE);
		}

		//the production/preproduction settings are now always disabled, period,
		//it is only so you can see your current status
		m_radioPreProduction.EnableWindow(FALSE);
		m_radioProduction.EnableWindow(FALSE);
		
	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// CNewCropSetupDlg message handlers

void CNewCropSetupDlg::OnOk()
{
	try {

		IRowSettingsPtr pRow = m_LocationCombo->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("You must have an Account location selected.");
			return;
		}
		long nLocationID = VarLong(pRow->GetValue(0));

		// (j.jones 2010-06-09 11:26) - PLID 39013 - removed all code that changed the production status,
		// it can no longer be done inside the program

		SetRemotePropertyInt("NewCropAccountLocationID", nLocationID, 0, "<None>");

		// (j.jones 2011-03-07 11:46) - PLID 42313 - added an option to not send patient gender
		SetRemotePropertyInt("NewCrop_DoNotSendPatientGender", m_checkSuppressPatGender.GetCheck() ? 1 : 0, 0, "<None>");

		CString strSqlBatch;
		CNxParamSqlArray args;
		
		strSqlBatch = BeginSqlBatch();

		// (j.jones 2011-06-17 11:12) - PLID 41709 - save the provider roles
		IRowSettingsPtr pProviderRoleRow = m_pProviderRoleList->GetFirstRow();
		while (pProviderRoleRow) {

			long nProviderID = VarLong(pProviderRoleRow->GetValue(prPersonID));
			NewCropProviderRoles ncprCurRole = (NewCropProviderRoles)VarLong(pProviderRoleRow->GetValue(prRoleTypeID), (long)ncprNone);

			_variant_t varRole = g_cvarNull;
			if(ncprCurRole != ncprNone) {
				varRole = (long)ncprCurRole;
			}

			AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE ProvidersT SET NewCropRole = {VT_I4} "
				"WHERE PersonID = {INT}", varRole, nProviderID);

			pProviderRoleRow = pProviderRoleRow->GetNextRow();
		}

		// (j.gruber 2009-03-31 09:29) - PLID 33328 - save the settings
		IRowSettingsPtr pUserTypeRow = m_pUserTypeList->GetFirstRow();		
		while (pUserTypeRow) {

			// (j.jones 2009-06-08 09:34) - PLID 34514 - added NewCropProviderID, which is required
			// if the user is a licensed prescriber

			long nUserTypeID = VarLong(pUserTypeRow->GetValue(utlcUserTypeID));
			_variant_t varLinkedProviderID = pUserTypeRow->GetValue(utlcLicensedPrescriberID);
			// (j.jones 2011-06-17 09:53) - PLID 41709 - added midlevel column
			_variant_t varMidlevelProviderID = pUserTypeRow->GetValue(utlcMidLevelProviderID);
			_variant_t varSupervisingProviderID = pUserTypeRow->GetValue(utlcSupervisingProviderID);

			// (j.jones 2009-08-18 17:13) - PLID 35203 - supported Midlevel Prescriber, which is handled by IsNewCropPrescriberRole()
			// (j.jones 2011-06-17 11:19) - PLID 41709 - midlevel now has its own column
			if((NewCropUserTypes)nUserTypeID == ncutLicensedPrescriber) {
				//require that a licensed prescriber is selected
				if(VarLong(varLinkedProviderID, -1) == -1) {
					AfxMessageBox("At least one user is configured to be a Licensed Prescriber but has no Licensed Prescriber selected for the user.\n\n"
						"All Licensed Prescriber users must have a Licensed Prescriber selected.");
					return;
				}
			}
			else {
				//ensure that the licensed prescriber field is cleared out
				varLinkedProviderID = g_cvarNull;
			}

			if((NewCropUserTypes)nUserTypeID == ncutMidlevelProvider) {
				//require that a midlevel provider is selected
				if(VarLong(varMidlevelProviderID, -1) == -1) {
					AfxMessageBox("At least one user is configured to be a Midlevel Provider but has no Midlevel Provider selected for the user.\n\n"
						"All Midlevel Provider users must have a Midlevel Provider selected.");
					return;
				}
			}
			else {
				//ensure that the midlevel provider field is cleared out
				varMidlevelProviderID = g_cvarNull;
			}

			// (j.jones 2009-08-25 08:39) - PLID 35203 - 
			if(IsNewCropSupervisedRole((NewCropUserTypes)nUserTypeID)) {
				//if a supervised role, and a supervising provider is chosen,
				//make sure it is NOT the same provider as the linked provider
				if(VarLong(varLinkedProviderID, -1) != -1
					&& VarLong(varSupervisingProviderID, -1) != -1
					&& VarLong(varLinkedProviderID, -1) == VarLong(varSupervisingProviderID, -1)) {
						
					AfxMessageBox("At least one user has a Supervising Provider that is identical to their Linked Provider.\n\n"
						"All users that have a Supervising Provider selected must have a different Supervising Provider than their Linked Provider.");
					return;
				}
			}

			//for display purposes, these really can be -1 sometimes,
			//and need to be reset to NULL
			if(VarLong(varLinkedProviderID, -1) == -1) {
				varLinkedProviderID = g_cvarNull;
			}

			// (j.jones 2011-06-17 10:57) - PLID 41709 - added NewCropMidlevelProviderID
			if(VarLong(varMidlevelProviderID, -1) == -1) {
				varMidlevelProviderID = g_cvarNull;
			}

			if(VarLong(varSupervisingProviderID, -1) == -1) {
				varSupervisingProviderID = g_cvarNull;
			}

			// (j.jones 2009-08-24 17:38) - PLID 35203 - added NewCropSupervisingProviderID
			// (j.jones 2011-06-17 10:57) - PLID 41709 - added NewCropMidlevelProviderID
			AddParamStatementToSqlBatch(strSqlBatch, args, "UPDATE UsersT SET NewCropUserTypeID = {INT}, "
				"NewCropProviderID = {VT_I4}, NewCropMidlevelProviderID = {VT_I4}, NewCropSupervisingProviderID = {VT_I4} "
				"WHERE PersonID = {INT} ",
				nUserTypeID, varLinkedProviderID, varMidlevelProviderID, varSupervisingProviderID,
				VarLong(pUserTypeRow->GetValue(utlcPersonID)));

			pUserTypeRow = pUserTypeRow->GetNextRow();
		}

		if (!strSqlBatch.IsEmpty()) {
			// (e.lally 2009-06-21) PLID 34680 - Leave as execute batch
			ExecuteParamSqlBatch(GetRemoteData(), strSqlBatch, args);
		}

		// (j.jones 2011-06-20 11:15) - PLID 44127 - Now that we have saved, validate all
		// the licensed prescribers' DEA numbers to make sure that two providers with a 
		// licensed prescriber role has the same DEA number.
		CString strInvalidProviders = "";
		if(!EnsureUniqueDEANumber(strInvalidProviders)) {
			CString strWarning;
			strWarning.Format("You have configured multiple providers that have the same DEA Number to use the Licensed Prescriber role:\n\n"
				"%s\n"
				"You cannot have two providers with the same DEA Number configured with the Licensed Prescriber role.\n\n"
				"You will need to change either their provider role, or their DEA Number in the Contacts module "
				"to ensure that no Licensed Prescribers have duplicate DEA Numbers.\n\n"
				"Do you wish to stay in the E-Prescribing setup and correct this now?", strInvalidProviders);

			if(IDYES == MessageBox(strWarning, "Practice", MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
		}

		CNxDialog::OnOK();
		
	}NxCatchAll("Error in CNewCropBrowserDlg::OnOk");
}

BEGIN_EVENTSINK_MAP(CNewCropSetupDlg, CNxDialog)
	ON_EVENT(CNewCropSetupDlg, IDC_NEWCROP_SETUP_USER_LIST, 10, OnEditingFinishedNewcropSetupUserList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNewCropSetupDlg, IDC_NEWCROP_SETUP_USER_LIST, 8, EditingStartingNewcropSetupUserList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CNewCropSetupDlg, IDC_NEWCROP_SETUP_PROVIDER_ROLE_LIST, 8, OnEditingStartingProviderRoleList, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CNewCropSetupDlg, IDC_NEWCROP_SETUP_PROVIDER_ROLE_LIST, 10, OnEditingFinishedProviderRoleList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)	
END_EVENTSINK_MAP()

// (j.jones 2011-06-17 10:05) - PLID 41709 - added provider role list
void CNewCropSetupDlg::OnEditingStartingProviderRoleList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		if(nCol == prRoleTypeID) {

			//if the current provider is in use in the user roles, warn before letting them change it
			NewCropProviderRoles ncprCurRole = (NewCropProviderRoles)VarLong(pRow->GetValue(prRoleTypeID), (long)ncprNone);
			if(ncprCurRole != ncprNone) {

				//see if this provider is chosen for this role in the user list

				long nProviderID = VarLong(pRow->GetValue(prPersonID));

				if(ncprCurRole == ncprLicensedPrescriber) {

					//see if this provider is in use as a licensed or supervising provider
					IRowSettingsPtr pUserRow = m_pUserTypeList->FindByColumn(utlcLicensedPrescriberID, (long)nProviderID, m_pUserTypeList->GetFirstRow(), FALSE);
					BOOL bFound = FALSE;
					if(pUserRow != NULL) {
						bFound = TRUE;
					}
					else {
						//try the supervising provider
						pUserRow = m_pUserTypeList->FindByColumn(utlcSupervisingProviderID, (long)nProviderID, m_pUserTypeList->GetFirstRow(), FALSE);
						if(pUserRow != NULL) {
							bFound = TRUE;
						}
					}

					if(bFound &&
						IDNO == MessageBox("This provider is assigned to a user role as a licensed prescriber or supervising provider. "
							"If you change this provider's role, they will be unlinked from these users.\n\n"
							"Are you sure you wish to change this provider's role?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
						*pbContinue = FALSE;
						return;
					}
				}
				else if(ncprCurRole == ncprMidlevelProvider) {

					//see if this provider is in use as a midlevel provider
					IRowSettingsPtr pUserRow = m_pUserTypeList->FindByColumn(utlcMidLevelProviderID, (long)nProviderID, m_pUserTypeList->GetFirstRow(), FALSE);
					if(pUserRow != NULL) {
						if(IDNO == MessageBox("This provider is assigned to a user role as a midlevel provider. "
							"If you change this provider's role, they will be unlinked from these users.\n\n"
							"Are you sure you wish to change this provider's role?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
							*pbContinue = FALSE;
							return;
						}
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-06-17 10:05) - PLID 41709 - added provider role list
void CNewCropSetupDlg::OnEditingFinishedProviderRoleList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if (pRow == NULL) {
			return;
		}

		//we only have one editable column
		if(nCol != prRoleTypeID) {
			return;
		}

		NewCropProviderRoles ncprOldRole = (NewCropProviderRoles)VarLong(varOldValue, (long)ncprNone);
		NewCropProviderRoles ncprCurRole = (NewCropProviderRoles)VarLong(varNewValue, (long)ncprNone);

		if(ncprOldRole == ncprCurRole) {
			//nothing changed
			return;
		}

		long nProviderID = VarLong(pRow->GetValue(prPersonID));

		//if the new role is not a licensed prescriber, make sure this provider
		//is not selected as any licensed or supervising provider in a user role
		if(ncprCurRole != ncprLicensedPrescriber) {
			long nCountFound = 0;

			IRowSettingsPtr pUserRow = m_pUserTypeList->GetFirstRow();
			while(pUserRow) {

				NewCropUserTypes eUserTypeID = (NewCropUserTypes)VarLong(pUserRow->GetValue(utlcUserTypeID), (long)ncutNone);

				if(VarLong(pUserRow->GetValue(utlcLicensedPrescriberID), -1) == nProviderID
					|| VarLong(pUserRow->GetValue(utlcSupervisingProviderID), -1) == nProviderID) {
					nCountFound++;
				}

				if(VarLong(pUserRow->GetValue(utlcLicensedPrescriberID), -1) == nProviderID) {
					//this provider is selected as a licensed provider but no longer
					//is eligible for such a role, so remove it

					//if the user type is licensed prescriber, set to none (-1), else null
					if(eUserTypeID == ncutLicensedPrescriber) {
						pUserRow->PutValue(utlcLicensedPrescriberID, (long)-1);
					}
					else {
						pUserRow->PutValue(utlcLicensedPrescriberID, g_cvarNull);
					}
				}
				if(VarLong(pUserRow->GetValue(utlcSupervisingProviderID), -1) == nProviderID) {
					//this provider is selected as a supervising provider but no longer
					//is eligible for such a role, so remove it

					//if the user type can have a supervising provider, set to none (-1), else null
					if(IsNewCropSupervisedRole(eUserTypeID)) {
						pUserRow->PutValue(utlcSupervisingProviderID, (long)-1);
					}
					else {
						pUserRow->PutValue(utlcSupervisingProviderID, g_cvarNull);
					}
				}

				pUserRow = pUserRow->GetNextRow();
			}

			//warn if we changed any users
			if(nCountFound > 0) {
				CString strWarn;
				strWarn.Format("%li %s had this provider removed from their user role. "
					"Please review the user roles list and make corrections as needed.", nCountFound, nCountFound > 1 ? "users have" : "user has");
				AfxMessageBox(strWarn);
			}
		}
		//if the new role is not a midlevel provider, make sure this provider
		//is not selected as any midlevel provider in a user role
		if(ncprCurRole != ncprMidlevelProvider) {
			long nCountFound = 0;

			IRowSettingsPtr pUserRow = m_pUserTypeList->GetFirstRow();
			while(pUserRow) {

				NewCropUserTypes eUserTypeID = (NewCropUserTypes)VarLong(pUserRow->GetValue(utlcUserTypeID), (long)ncutNone);

				if(VarLong(pUserRow->GetValue(utlcMidLevelProviderID), -1) == nProviderID) {
					//this provider is selected as a midlevel provider but no longer
					//is eligible for such a role, so remove it

					nCountFound++;

					//if the user type is midlevel provider, set to none (-1), else null
					if(eUserTypeID == ncutMidlevelProvider) {
						pUserRow->PutValue(utlcMidLevelProviderID, (long)-1);
					}
					else {
						pUserRow->PutValue(utlcMidLevelProviderID, g_cvarNull);
					}
				}

				pUserRow = pUserRow->GetNextRow();
			}

			//warn if we changed any users
			if(nCountFound > 0) {
				CString strWarn;
				strWarn.Format("%li %s had this provider removed from their user role. "
					"Please review the user roles list and make corrections as needed.", nCountFound, nCountFound > 1 ? "users have" : "user has");
				AfxMessageBox(strWarn);
			}
		}

		//reload the provider combos in the user list
		RequeryProviderComboColumns();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2009-06-08 09:39) - PLID 34514 - added OnEditingFinishedNewcropSetupUserList
void CNewCropSetupDlg::OnEditingFinishedNewcropSetupUserList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == utlcUserTypeID) {
			//if the user type is not a licensed prescriber, ensure the provider field is cleared out
			// (j.jones 2009-08-18 17:13) - PLID 35203 - supported Midlevel Prescriber, which is handled by IsNewCropPrescriberRole()
			// (j.jones 2011-06-17 10:50) - PLID 41709 - split licensed & midlevel to separate columns
			if((NewCropUserTypes)VarLong(varNewValue) != ncutLicensedPrescriber) {
				pRow->PutValue(utlcLicensedPrescriberID, g_cvarNull);
			}
			else if(pRow->GetValue(utlcLicensedPrescriberID).vt != VT_I4) {
				//otherwise if the field is already null, set it as -1,
				//so we can view the "unselected" row
				pRow->PutValue(utlcLicensedPrescriberID, (long)-1);
			}

			if((NewCropUserTypes)VarLong(varNewValue) != ncutMidlevelProvider) {
				pRow->PutValue(utlcMidLevelProviderID, g_cvarNull);
			}
			else if(pRow->GetValue(utlcMidLevelProviderID).vt != VT_I4) {
				//otherwise if the field is already null, set it as -1,
				//so we can view the "unselected" row
				pRow->PutValue(utlcMidLevelProviderID, (long)-1);
			}

			// (j.jones 2009-08-25 08:34) - PLID 35203 - this cannot be an else if from above,
			// because Midlevel Prescribers are both prescribers AND supervised
			if(!IsNewCropSupervisedRole((NewCropUserTypes)VarLong(varNewValue))) {
				pRow->PutValue(utlcSupervisingProviderID, g_cvarNull);
			}
			else if(pRow->GetValue(utlcSupervisingProviderID).vt != VT_I4) {
				//otherwise if the field is already null, set it as -1,
				//so we can view the "unselected" row
				pRow->PutValue(utlcSupervisingProviderID, (long)-1);
			}
		}

	}NxCatchAll("Error in CNewCropBrowserDlg::OnEditingFinishedNewcropSetupUserList");
}

void CNewCropSetupDlg::EditingStartingNewcropSetupUserList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try {

		// (j.gruber 2009-06-10 11:56) - PLID 34515 - don't let them edit the provider column if its not licensed prescriber
		IRowSettingsPtr pRow(lpRow);
		if (pRow) {

			if(nCol == utlcLicensedPrescriberID) {

				// (j.jones 2011-06-17 09:55) - PLID 41709 - you can only edit this column
				// if the user is a licensed prescriber
				if((NewCropUserTypes)VarLong(pRow->GetValue(utlcUserTypeID), (long)ncutNone) != ncutLicensedPrescriber) {
					//don't let them edit it
					*pbContinue = FALSE;
				}
			}
			// (j.jones 2011-06-17 09:53) - PLID 41709 - added midlevel column
			else if(nCol == utlcMidLevelProviderID) {
				//you can only edit this column if the user is a midlevel provider
				if((NewCropUserTypes)VarLong(pRow->GetValue(utlcUserTypeID), (long)ncutNone) != ncutMidlevelProvider) {
					//don't let them edit it
					*pbContinue = FALSE;
				}
			}
			else if(nCol == utlcSupervisingProviderID) {

				//check to see what the role is
				// (j.jones 2009-08-25 08:34) - PLID 35203 - staff and midlevel prescribers are supervised
				if(!IsNewCropSupervisedRole((NewCropUserTypes)VarLong(pRow->GetValue(utlcUserTypeID), (long)ncutNone))) {
					//don't let them edit it
					*pbContinue = FALSE;
				}
			}
		}

	}NxCatchAll("Error in CNewCropSetupDlg::EditingStartingNewcropSetupUserList");
}

// (j.jones 2011-06-17 08:35) - PLID 44157 - moved the name overrides to a new dialog
void CNewCropSetupDlg::OnBtnNewcropOverrideNames()
{
	try {

		CNewCropNameOverridesDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll(__FUNCTION__);
}

// (j.jones 2011-06-17 11:29) - PLID 41709 - added a function to re-generate
// the embedded combo boxes for each provider dropdown column, while
// maintaining their existing selections
void CNewCropSetupDlg::RequeryProviderComboColumns()
{
	//this function WILL remove inactive providers from the list
	//and intentionally not re-select them, since now they have
	//actively changed who is a viable provider

	//first build our provider combo info.
	
	CString strLicensedPrescribers = "-1;<None>;";
	CString strMidlevelProviders = "-1;<None>;";
	CString strSupervisingProviders = "-1;<Prompt Per Login>;";

	IRowSettingsPtr pProviderRoleRow = m_pProviderRoleList->GetFirstRow();
	while(pProviderRoleRow) {
		long nProviderID = VarLong(pProviderRoleRow->GetValue(prPersonID));
		CString strProviderName = VarString(pProviderRoleRow->GetValue(prName), "");
		NewCropProviderRoles ncprCurRole = (NewCropProviderRoles)VarLong(pProviderRoleRow->GetValue(prRoleTypeID), (long)ncprNone);

		strProviderName.Replace(";"," ");

		CString strEntry;
		strEntry.Format("%li;%s;", nProviderID, strProviderName);
		
		if(ncprCurRole == ncprLicensedPrescriber) {
			strLicensedPrescribers += strEntry;
			//licensed prescribers can be supervising as well
			strSupervisingProviders += strEntry;
		}
		else if(ncprCurRole == ncprMidlevelProvider) {
			strMidlevelProviders += strEntry;
		}

		pProviderRoleRow = pProviderRoleRow->GetNextRow();
	}

	//before we replace our combo text, cache the old values
	CMap<long, long, _variant_t, _variant_t> mapUserIDToLicensedPrescriberVal;
	CMap<long, long, _variant_t, _variant_t> mapUserIDToMidlevelProviderVal;
	CMap<long, long, _variant_t, _variant_t> mapUserIDToSupervisingProviderVal;

	IRowSettingsPtr pUserTypeRow = m_pUserTypeList->GetFirstRow();		
	while(pUserTypeRow) {

		long nUserID = VarLong(pUserTypeRow->GetValue(utlcPersonID));
		_variant_t varLinkedProviderID = pUserTypeRow->GetValue(utlcLicensedPrescriberID);
		_variant_t varMidlevelProviderID = pUserTypeRow->GetValue(utlcMidLevelProviderID);
		_variant_t varSupervisingProviderID = pUserTypeRow->GetValue(utlcSupervisingProviderID);

		mapUserIDToLicensedPrescriberVal.SetAt(nUserID, varLinkedProviderID);
		mapUserIDToMidlevelProviderVal.SetAt(nUserID, varMidlevelProviderID);
		mapUserIDToSupervisingProviderVal.SetAt(nUserID, varSupervisingProviderID);

		pUserTypeRow = pUserTypeRow->GetNextRow();
	}

	//now reset our combo contents
	IColumnSettingsPtr pLinkedProvCol, pMidlevelProvCol, pSupervisingProvCol;
	pLinkedProvCol = m_pUserTypeList->GetColumn(utlcLicensedPrescriberID);
	pMidlevelProvCol = m_pUserTypeList->GetColumn(utlcMidLevelProviderID);
	pSupervisingProvCol = m_pUserTypeList->GetColumn(utlcSupervisingProviderID);

	pLinkedProvCol->PutComboSource(_bstr_t(strLicensedPrescribers));
	pMidlevelProvCol->PutComboSource(_bstr_t(strMidlevelProviders));
	pSupervisingProvCol->PutComboSource(_bstr_t(strSupervisingProviders));

	//and finally, restore our old selections
	pUserTypeRow = m_pUserTypeList->GetFirstRow();		
	while(pUserTypeRow) {

		long nUserID = VarLong(pUserTypeRow->GetValue(utlcPersonID));
		
		_variant_t varLinkedProviderID = g_cvarNull;
		_variant_t varMidlevelProviderID = g_cvarNull;
		_variant_t varSupervisingProviderID = g_cvarNull;

		mapUserIDToLicensedPrescriberVal.Lookup(nUserID, varLinkedProviderID);
		mapUserIDToMidlevelProviderVal.Lookup(nUserID, varMidlevelProviderID);
		mapUserIDToSupervisingProviderVal.Lookup(nUserID, varSupervisingProviderID);
		
		pUserTypeRow->PutValue(utlcLicensedPrescriberID, varLinkedProviderID);
		pUserTypeRow->PutValue(utlcMidLevelProviderID, varMidlevelProviderID);
		pUserTypeRow->PutValue(utlcSupervisingProviderID, varSupervisingProviderID);

		pUserTypeRow = pUserTypeRow->GetNextRow();
	}
}