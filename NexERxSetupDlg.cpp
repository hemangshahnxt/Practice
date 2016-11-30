// NexERxSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "NexERxSetupDlg.h"
#include "SingleSelectDlg.h"
#include "MultiSelectDlg.h"
#include <foreach.h>
#include "NexERxConfigureLicenses.h"
#include "PrescriptionUtilsNonAPI.h"	// (j.jones 2013-03-27 17:23) - PLID 55920 - we only need the non-API header here

// CNexERxSetupDlg dialog
// (b.savon 2013-01-11 10:00) - PLID 54578 - Created

enum EUserCellColor {
	uccInvalid = RGB(238,238,224),
	uccValid = RGB(152,251,152),
	uccIncomplete = RGB(255,127,127),
};

IMPLEMENT_DYNAMIC(CNexERxSetupDlg, CNxDialog)

CNexERxSetupDlg::CNexERxSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CNexERxSetupDlg::IDD, pParent)
{

}

CNexERxSetupDlg::~CNexERxSetupDlg()
{
}

void CNexERxSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_NXC_PROVIDER_BACK, m_nxcProviderBack);
	DDX_Control(pDX, IDC_NXC_USER_BACK, m_nxcUserBack);
	DDX_Control(pDX, IDC_BTN_CONFIGURE_NEXERX_LICENSES, m_btnConfigureNexERxLicenses);
}

BOOL CNexERxSetupDlg::OnInitDialog()
{
	CNxDialog::OnInitDialog();

	try{

		// (b.savon 2013-01-11 10:11) - PLID 54578 - Dialog Setup
		SetTitleBarIcon(IDI_ERX);
		SetMinSize(850, 550);
		EnableDragHandle();

		m_nxcProviderBack.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_nxcUserBack.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnConfigureNexERxLicenses.AutoSet(NXB_PILLBOTTLE);
		
		m_nxdlProviderRole = BindNxDataList2Ctrl(IDC_NXDL_PROVIDER_ROLE);
		m_nxdlUserRole = BindNxDataList2Ctrl(IDC_NXDL_USER_ROLES, false);

		LoadUserRoles();

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

// (b.savon 2013-01-11 10:11) - PLID 54578 - Handles the coloring and cell formatting for the user roles section
void CNexERxSetupDlg::SetUserRolesCellProperties()
{
	// Standard, disabled cell
	NXDATALIST2Lib::IFormatSettingsPtr pNoLinkNoEdit(__uuidof(NXDATALIST2Lib::FormatSettings));
	pNoLinkNoEdit->PutFieldType(NXDATALIST2Lib::cftTextSingleLine);
	pNoLinkNoEdit->PutEditable(VARIANT_FALSE);

	// Cell that can be clicked for a wizard
	NXDATALIST2Lib::IFormatSettingsPtr pLinkEdit(__uuidof(NXDATALIST2Lib::FormatSettings));
	pLinkEdit->PutFieldType(NXDATALIST2Lib::cftTextWordWrapLink);
	pLinkEdit->PutEditable(VARIANT_TRUE);

	// Go through the rows, Get the user type, and format the cells in that row as they should be
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserRole->GetFirstRow();
	while(pRow){
		// Get the user type f or the current row
		EUserRoleTypes urtCurrentRow = (EUserRoleTypes)VarLong(pRow->GetValue(urcUserType), urtNone);
		switch( urtCurrentRow ){
			case urtNone:
				{
					pRow->PutRefCellFormatOverride(urcLicensedPrescriber, pNoLinkNoEdit);
					pRow->PutRefCellFormatOverride(urcMidlevelPrescriber, pNoLinkNoEdit);
					pRow->PutRefCellFormatOverride(urcSupervisor, pNoLinkNoEdit);
					pRow->PutRefCellFormatOverride(urcPrescribers, pNoLinkNoEdit);

					pRow->PutCellBackColor(urcLicensedPrescriber, uccInvalid);
					pRow->PutCellBackColor(urcMidlevelPrescriber, uccInvalid);
					pRow->PutCellBackColor(urcSupervisor, uccInvalid);
					pRow->PutCellBackColor(urcPrescribers, uccInvalid);
				}
				break;
			case urtLicensedPrescriber:
				{
					pRow->PutRefCellFormatOverride(urcLicensedPrescriber, pLinkEdit);
					pRow->PutRefCellFormatOverride(urcMidlevelPrescriber, pNoLinkNoEdit);
					pRow->PutRefCellFormatOverride(urcSupervisor, pNoLinkNoEdit);
					pRow->PutRefCellFormatOverride(urcPrescribers, pNoLinkNoEdit);

					// If the user is configured to be a licensed prescriber and they have a prescriber id assigned, make the cell color valid (Green),
					// otherwise, make it invalid (red).
					long nLicensedPrescriber;
					pRow->PutCellBackColor(urcLicensedPrescriber,  GetLicensedPrescriberID(pRow, nLicensedPrescriber) ? uccValid : uccIncomplete );
					pRow->PutCellBackColor(urcMidlevelPrescriber, uccInvalid);
					pRow->PutCellBackColor(urcSupervisor, uccInvalid);
					pRow->PutCellBackColor(urcPrescribers, uccInvalid);
				}
				break;
			case urtMidlevelPrescriber:
				{
					pRow->PutRefCellFormatOverride(urcLicensedPrescriber, pNoLinkNoEdit);
					pRow->PutRefCellFormatOverride(urcMidlevelPrescriber, pLinkEdit);
					pRow->PutRefCellFormatOverride(urcSupervisor, pLinkEdit);
					pRow->PutRefCellFormatOverride(urcPrescribers, pNoLinkNoEdit);

					// If the user is configured to be a midlevel prescriber and they have prescriber id(s) assigned, make the cell color valid (Green),
					// otherwise, make it invalid (red).
					long nMidlevel;
					CArray<long, long> arySupervisor;
					pRow->PutCellBackColor(urcLicensedPrescriber, uccInvalid);
					pRow->PutCellBackColor(urcMidlevelPrescriber, GetMidlevelPrescriberID(pRow, nMidlevel) ? uccValid : uccIncomplete);
					pRow->PutCellBackColor(urcSupervisor, GetSupervisingIDs(pRow, arySupervisor) ? uccValid : uccIncomplete);
					pRow->PutCellBackColor(urcPrescribers, uccInvalid);
				}
				break;
			case urtNurseStaff:
				{
					pRow->PutRefCellFormatOverride(urcLicensedPrescriber, pNoLinkNoEdit);
					pRow->PutRefCellFormatOverride(urcMidlevelPrescriber, pNoLinkNoEdit);
					pRow->PutRefCellFormatOverride(urcSupervisor, pNoLinkNoEdit);
					pRow->PutRefCellFormatOverride(urcPrescribers, pLinkEdit);

					// If the user is configured to be a nurse staff and they have prescriber id(s) assigned, make the cell color valid (Green),
					// otherwise, make it invalid (red).
					CSimpleArray<long> arySupervising;
					CSimpleArray<long> aryMidlevel;
					pRow->PutCellBackColor(urcLicensedPrescriber, uccInvalid);
					pRow->PutCellBackColor(urcMidlevelPrescriber, uccInvalid);
					pRow->PutCellBackColor(urcSupervisor, uccInvalid);
					pRow->PutCellBackColor(urcPrescribers, GetNurseStaffPrescriberIDs(pRow, arySupervising, aryMidlevel) ? uccValid : uccIncomplete);
				}
				break;
		}

		pRow = pRow->GetNextRow();
	}
}


BEGIN_MESSAGE_MAP(CNexERxSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CNexERxSetupDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_NEXERX_LICENSES, &CNexERxSetupDlg::OnBnClickedBtnConfigureLicenses)
	ON_WM_CLOSE()
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CNexERxSetupDlg message handlers
BEGIN_EVENTSINK_MAP(CNexERxSetupDlg, CNxDialog)
	ON_EVENT(CNexERxSetupDlg, IDC_NXDL_USER_ROLES, 10, CNexERxSetupDlg::EditingFinishedNxdlUserRoles, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNexERxSetupDlg, IDC_NXDL_PROVIDER_ROLE, 10, CNexERxSetupDlg::EditingFinishedNxdlProviderRole, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CNexERxSetupDlg, IDC_NXDL_USER_ROLES, 19, CNexERxSetupDlg::LeftClickNxdlUserRoles, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CNexERxSetupDlg, IDC_NXDL_PROVIDER_ROLE, 8, CNexERxSetupDlg::EditingStartingNxdlProviderRole, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
	ON_EVENT(CNexERxSetupDlg, IDC_NXDL_USER_ROLES, 8, CNexERxSetupDlg::EditingStartingNxdlUserRoles, VTS_DISPATCH VTS_I2 VTS_PVARIANT VTS_PBOOL)
END_EVENTSINK_MAP()

void CNexERxSetupDlg::LoadUserRoles()
{
	try{

		// (b.savon 2013-01-11 10:15) - PLID 54581 - Handle the loading of the licensed prescriber
		// (b.savon 2013-01-11 10:27) - PLID 54583 - Handle the loading of the midlevel
		// (b.savon 2013-01-11 10:34) - PLID 54584 - Handle the loading of the Nurse/Staff
		ADODB::_RecordsetPtr prs = CreateParamRecordset("{SQL}", GetNexERxUserRoles(-1));

		NXDATALIST2Lib::IColumnSettingsPtr csp = m_nxdlUserRole->GetColumn(urcUserType);
		csp->PutComboSource(_bstr_t("-1;<None>;0;Licensed Prescriber;1;Midlevel Provider;2;Nurse/Staff"));

		while(!prs->eof){
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserRole->GetNewRow();

			pRow->PutValue(urcID, AdoFldLong(prs->Fields, "ID")); //Everyone has an ID
			pRow->PutValue(urcUserName, _bstr_t(AdoFldString(prs->Fields, "UserName"))); //Everyone has a UserName
			pRow->PutValue(urcUserType, AdoFldLong(prs->Fields, "UserType", urtNone));
			// (b.savon 2013-01-11 10:15) - PLID 54581 - Load the licensed prescriber
			pRow->PutValue(urcLicensedPrescriberID, prs->Fields->Item["LicensedPrescriberID"]->Value);
			pRow->PutValue(urcLicensedPrescriber, prs->Fields->Item["LicensedPrescriberName"]->Value);
			// (b.savon 2013-01-11 10:27) - PLID 54583 - Load the midlevel
			pRow->PutValue(urcMidlevelPrescriberID, prs->Fields->Item["MidlevelPrescriberID"]->Value);
			pRow->PutValue(urcMidlevelPrescriber, prs->Fields->Item["MidlevelPrescriberName"]->Value);
			pRow->PutValue(urcSupervisorIDs, prs->Fields->Item["SupervisingIDs"]->Value);
			pRow->PutValue(urcSupervisor, prs->Fields->Item["SupervisingNames"]->Value);
			// (b.savon 2013-01-11 10:34) - PLID 54584 - Load the Nurse/Staff
			pRow->PutValue(urcPrescriberIDs, prs->Fields->Item["PrescribingIDs"]->Value);
			pRow->PutValue(urcPrescribers, prs->Fields->Item["PrescribingNames"]->Value);

			m_nxdlUserRole->AddRowSorted(pRow, NULL);

			prs->MoveNext();
		}

		SetUserRolesCellProperties();

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-11 10:06) - PLID 54580 - Save the provider role if changed
void CNexERxSetupDlg::EditingFinishedNxdlProviderRole(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		// If the user didn't escape
		if(bCommit){
			//If the row is valid
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if( pRow ){
				// If were in the provider type column
				if( nCol == prcProviderRoleID ){
					// If the value changed
					EProviderRoleTypes prtOldRole = (EProviderRoleTypes)VarLong(varOldValue, (long)prtNone);
					EProviderRoleTypes prtNewRole = (EProviderRoleTypes)VarLong(varNewValue, (long)prtNone);
					if( prtOldRole != prtNewRole ){
						long nProviderID = VarLong(pRow->GetValue(prcPersonID));		
						if( prtNewRole == prtNone ){ // They already said go for it, so remove all the providers user roles to none that have the providerid
							UnregisterPrescriber(nProviderID);
						}
						switch( prtNewRole ){
							case prtLicensedPrescriber:
								{
									if( GetCountProviderRole(prtLicensedPrescriber) > m_lNexERxLicense.GetAllowedLicensedPrescriberCount() ){
										MessageBox("You have surpassed the number of allowed Licensed Prescribers.  Please remove any unneeded Licensed Prescribers or call your account manager to request more licenses.",
													"Licensed Prescriber License", MB_ICONINFORMATION);
										pRow->PutValue(prcProviderRoleID, prtOldRole);
										return;
									}
								}
								break;
							case prtMidlevelPrescriber:
								{
									if( GetCountProviderRole(prtMidlevelPrescriber) > m_lNexERxLicense.GetAllowedMidlevelPrescriberCount() ){
										MessageBox("You have surpassed the number of allowed Midlevel Prescribers.  Please remove any unneeded Midlevel Prescribers or call your account manager to request more licenses.",
													"Midlevel Prescriber License", MB_ICONINFORMATION);
										pRow->PutValue(prcProviderRoleID, prtOldRole);
										return;
									}
								}
								break;
						}
						ExecuteParamSql("UPDATE ProvidersT SET NexERxProviderTypeID = {INT} WHERE PersonID = {INT}", (long)prtNewRole, nProviderID);
					}
				}
			}
		}
		

	}NxCatchAll(__FUNCTION__);
}

void CNexERxSetupDlg::EditingFinishedNxdlUserRoles(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try{
		// If the user didn't escape
		if(bCommit){
			// If the row is valid
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			if( pRow ){
				// If we're in the user type column
				if( nCol == urcUserType ){
					// If the value changed
					EUserRoleTypes urtOldRole = (EUserRoleTypes)VarLong(varOldValue, (long)urtNone);
					EUserRoleTypes urtCurRole = (EUserRoleTypes)VarLong(varNewValue, (long)urtNone);
					if( urtOldRole != urtCurRole ){
						long nUserID = VarLong(pRow->GetValue(urcID), -1);
						// Check if the user was assigned a Nurse/Staff role
						if( urtCurRole == urtNurseStaff ){
							if( GetCountUserRole(urtNurseStaff) > m_lNexERxLicense.GetAllowedNurseStaffCount() ){
								MessageBox("You have surpassed the number of allowed Nurse/Staff users.  Please remove any unneeded Nurse/Staff users or call your account manager to purchase more licenses.",
											"Nurse/Staff User License", MB_ICONINFORMATION);
								pRow->PutValue(urcUserType, urtOldRole);
								return;
							}
						}
						SaveUserRole(pRow, urtCurRole);
					}
				}
			}
		}

	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2013-01-11 10:16) - PLID 54581 - Clear
void CNexERxSetupDlg::ClearLicensedPrescriber(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// The caller should be responsible for a valid row, but let's be safe
	if( pRow ){
		pRow->PutValue(urcLicensedPrescriberID, g_cvarNull);
		pRow->PutValue(urcLicensedPrescriber, _variant_t(_bstr_t("")));
	}
}

// (b.savon 2013-01-11 10:16) - PLID 54581 - Put
void CNexERxSetupDlg::PutLicensedPrescriber(NXDATALIST2Lib::IRowSettingsPtr pRow, LicensedPrescriberReturn lprLicensed)
{
	// The caller should be responsible for a valid row, but let's be safe
	if( pRow ){
		if( lprLicensed.bCancelled ){
			pRow->PutCellBackColor(urcLicensedPrescriber, uccIncomplete);
		}else{
			if( IsValidPrescriberSelection(lprLicensed.nID, urcLicensedPrescriberID) ){
				pRow->PutValue(urcLicensedPrescriberID, (long)lprLicensed.nID);
				pRow->PutValue(urcLicensedPrescriber, _variant_t(_bstr_t(lprLicensed.strFullName)));
				pRow->PutCellBackColor(urcLicensedPrescriber, uccValid);
			}else{
				MessageBox("The Licensed Prescriber selected is already assigned to a user.  If the current user that is assigned is incorrect, please remove the selection and try again.",
						   "Invalid Licensed Prescriber Selection", MB_ICONINFORMATION);
				pRow->PutCellBackColor(urcLicensedPrescriber, uccIncomplete);
			}
		}
	}
}

// (b.savon 2013-01-11 10:28) - PLID 54583 - Clear
void CNexERxSetupDlg::ClearMidlevelPrescriber(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// The caller should be responsible for a valid row, but let's be safe
	if( pRow ){
		pRow->PutValue(urcMidlevelPrescriberID, g_cvarNull);
		pRow->PutValue(urcMidlevelPrescriber, _variant_t(_bstr_t("")));
	}
}

// (b.savon 2013-01-11 10:28) - PLID 54583 - Put
void CNexERxSetupDlg::PutMidlevelPrescriber(NXDATALIST2Lib::IRowSettingsPtr pRow, MidlevelPrescriberReturn mprLicensed)
{
	// The caller should be responsible for a valid row, but let's be safe
	if( pRow ){
		// If they cancelled, make the column red so they know that this user has incomplete data to fill in.
		if( mprLicensed.bCancelled ){
			pRow->PutCellBackColor(urcMidlevelPrescriber, uccIncomplete);
			pRow->PutCellBackColor(urcSupervisor, uccIncomplete);
		}else{
			if( IsValidPrescriberSelection(mprLicensed.nID, urcMidlevelPrescriberID) ){
				pRow->PutValue(urcMidlevelPrescriberID, (long)mprLicensed.nID);
				pRow->PutValue(urcMidlevelPrescriber, _variant_t(_bstr_t(mprLicensed.strFullName)));
				pRow->PutCellBackColor(urcMidlevelPrescriber, uccValid);
			}else{
				MessageBox("The Midlevel Prescriber selected is already assigned to a user.  If the current user that is assigned is incorrect, please remove the selection and try again.",
						   "Invalid Midlevel Prescriber Selection", MB_ICONINFORMATION);
				pRow->PutCellBackColor(urcMidlevelPrescriber, uccIncomplete);
			}
		}
	}
}

// (b.savon 2013-01-11 10:28) - PLID 54583 - Clear
void CNexERxSetupDlg::ClearSupervisor(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// The caller should be responsible for a valid row, but let's be safe
	if( pRow ){
		pRow->PutValue(urcSupervisorIDs, _variant_t(_bstr_t("")));
		pRow->PutValue(urcSupervisor, _variant_t(_bstr_t("")));
	}
}

// (b.savon 2013-01-11 10:28) - PLID 54583 - Put
void CNexERxSetupDlg::PutSupervisors(NXDATALIST2Lib::IRowSettingsPtr pRow, SupervisingReturn srSupervisor)
{
	// The caller should be responsible for a valid row, but let's be safe
	if( pRow ){
		CArray<long, long> aryIDs;
		if( srSupervisor.strNames.IsEmpty() && !GetSupervisingIDs(pRow, aryIDs) ){
			pRow->PutCellBackColor(urcSupervisor, uccIncomplete);
		}else if( !srSupervisor.bCancelled ){
			pRow->PutValue(urcSupervisorIDs, _variant_t(_bstr_t(srSupervisor.strIDs)));
			pRow->PutValue(urcSupervisor, _variant_t(_bstr_t(srSupervisor.strNames)));
			if( srSupervisor.strNames.IsEmpty() ){
				pRow->PutCellBackColor(urcSupervisor, uccIncomplete);
			}else{
				pRow->PutCellBackColor(urcSupervisor, uccValid);
			}
		}
	}
}

// (b.savon 2013-01-11 10:35) - PLID 54584 - Clear
void CNexERxSetupDlg::ClearPrescriber(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// The caller should be responsible for a valid row, but let's be safe
	if( pRow ){
		pRow->PutValue(urcPrescriberIDs, _variant_t(_bstr_t("")));
		pRow->PutValue(urcPrescribers, _variant_t(_bstr_t("")));
	}
}

// (b.savon 2013-01-11 10:35) - PLID 54584 - Put
void CNexERxSetupDlg::PutPrescribers(NXDATALIST2Lib::IRowSettingsPtr pRow, SupervisingReturn srSupervisor, MultiMidlevelPrescriberReturn mmprMidlevel, BOOL bOverride /*=FALSE*/)
{
	// The caller should be responsible for a valid row, but let's be safe
	if( pRow ){
		// Get the IDs and Names of the supervisors and midlevels (if any)
		CString strSupervising, strMidlevel;
		CString strSupervisingNames, strMidlevelNames;
		if( !bOverride ){
			GetNurseStaffMidlevelSupervisingIDs(pRow, strSupervising, strMidlevel);
			GetNurseStaffMidlevelSupervisingNames(pRow, strSupervisingNames, strMidlevelNames);
		}

		// When we put Prescribers for the Nurse/Staff we bundle them into the same column and delineate the ids
		// by a leading *S* for supervisor, and *M* for midlevel.  The caller passes us the Supervising and midlevel
		// structs that are constructed from the wizard.  If there are IDs from the wizard, use them.  If the user
		// cancelled the wizards and there were previous values in the cells, retain them.
		CString strIDs;
		if(!srSupervisor.strIDs.IsEmpty()){
			strIDs = "*S*" + srSupervisor.strIDs;
		}
		if(srSupervisor.bCancelled && !strSupervising.IsEmpty()){
			strIDs = "*S*" + strSupervising;
		}
		if(!mmprMidlevel.strIDs.IsEmpty()){
			strIDs += "*M*" + mmprMidlevel.strIDs;
		}
		if(mmprMidlevel.bCancelled && !strMidlevel.IsEmpty()){
			strIDs += "*M*" + strMidlevel;
		}

		// Likewise for the names.  This time we delineate the names by a leading "Supervisor(s): " or "Midlevel(s): " because
		// the user sees the names in the cells so we format them nicely.
		CString strNames;
		if( !srSupervisor.strNames.IsEmpty() ){
			strNames = "Supervisor(s): " + srSupervisor.strNames;
		}
		if( srSupervisor.bCancelled && !strSupervisingNames.IsEmpty() ){
			strNames = "Supervisor(s): " + strSupervisingNames;
		}
		if( !mmprMidlevel.strFullNames.IsEmpty() ){
			if( !strNames.IsEmpty() ){
				strNames += "; ";
			}
			strNames += "Midlevel(s): " + mmprMidlevel.strFullNames;	
		}
		if( mmprMidlevel.bCancelled && !strMidlevelNames.IsEmpty() ){
			if( !strNames.IsEmpty() ){
				strNames += "; ";
			}
			strNames += "Midlevel(s): " + strMidlevelNames;
		}

		// (b.savon 2013-01-11 10:41) - PLID 54584 - Format the cell colors correctly.
		if( srSupervisor.strNames.IsEmpty() && mmprMidlevel.strFullNames.IsEmpty() && strSupervising.IsEmpty() && strMidlevel.IsEmpty() ){
			pRow->PutCellBackColor(urcPrescribers, uccIncomplete);
			ClearPrescriber(pRow);
		}else if( !srSupervisor.bCancelled || !mmprMidlevel.bCancelled ){
			pRow->PutValue(urcPrescriberIDs, _variant_t(_bstr_t(strIDs)));
			pRow->PutValue(urcPrescribers, _variant_t(_bstr_t(strNames)));
			if( strNames.IsEmpty() ){
				pRow->PutCellBackColor(urcPrescribers, uccIncomplete);
			}else{
				pRow->PutCellBackColor(urcPrescribers, uccValid);
			}
		}
	}
}

// (b.savon 2013-01-11 10:17) - PLID 54581 - Licensed Prescriber Wizard
LicensedPrescriberReturn CNexERxSetupDlg::LicensedPrescriberWizard()
{
	// Wizard Steps
	//	1. Select the licensed prescriber
	return SelectLicensedPrescriber();

	// Done. That was easy.
}

// (b.savon 2013-01-11 10:17) - PLID 54581 - Select a licensed prescriber
LicensedPrescriberReturn CNexERxSetupDlg::SelectLicensedPrescriber()
{
	LicensedPrescriberReturn lprPrescriber;

	// Don't include deactivated
	CString strAdditionalWhere;
	CString strInactive = m_lNexERxLicense.GetInactiveLicensedPrescriberIDString();
	if( !strInactive.IsEmpty() ){
		strAdditionalWhere.Format(" AND PersonT.ID NOT IN (%s)", strInactive);
	}

	CSingleSelectDlg dlg(this);
	//prtLicensedPrescriber = 0
	if( IDOK == dlg.Open(
					/* FROM */
					"ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID", 
					/* WHERE */
					"NexERxProviderTypeID = 0 AND PersonT.Archived = 0" + strAdditionalWhere,
					/* ID */
					"PersonT.ID",
					/* Value */
					"PersonT.FullName",
					/* Description */
					"The list below contains all the providers that are categorized as a Licensed Prescriber in the Provider Roles section.  Please select the provider that is assigned to this username.",
					/* Force Selection */
					true) ){
		lprPrescriber.bCancelled = FALSE;
	}else{
		lprPrescriber.bCancelled = TRUE;
	}
	
	lprPrescriber.nID = dlg.GetSelectedID();
	lprPrescriber.strFullName = AsString(dlg.GetSelectedDisplayValue());

	return lprPrescriber;
}

// (b.savon 2013-01-11 10:29) - PLID 54583 - Midlevel wizard
void CNexERxSetupDlg::MidlevelPrescriberWizard(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// Wizard Steps
	//	1. Choose the Midlevel
	MidlevelPrescriberReturn mprMidlevel = SelectMidlevelPrescriber();
	// The caller should be responsible for passing in a valid row, but let's be safe.
	if( pRow ){
		PutMidlevelPrescriber(pRow, mprMidlevel);
	}

	// They must assign this user to a mid-level.  Don't let them continue if they cancel.
	if( mprMidlevel.bCancelled ){
		return;
	}
	

	//	2. Choose the supervisors
	SupervisingReturn srSupervisors = SelectSupervisors(pRow);
	//The caller should be responsible for passing in a valid row, but let's be safe;
	if( pRow ){
		PutSupervisors(pRow, srSupervisors);
	}
}

// (b.savon 2013-01-11 10:29) - PLID 54583 - Select Midlevel
MidlevelPrescriberReturn CNexERxSetupDlg::SelectMidlevelPrescriber()
{
	MidlevelPrescriberReturn mprPrescriber;

	// Don't include deactivated
	CString strAdditionalWhere = m_lNexERxLicense.GetInactiveMidlevelPrescriberIDString();
	if( !strAdditionalWhere.IsEmpty() ){
		strAdditionalWhere.Format(" AND PersonT.ID NOT IN (%s)", strAdditionalWhere);
	}
	CSingleSelectDlg dlg(this);
	//prtMidlevelPrescriber = 1
	if( IDOK == dlg.Open(
					/* FROM */
					"ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID", 
					/* WHERE */
					"NexERxProviderTypeID = 1 AND PersonT.Archived = 0" + strAdditionalWhere,
					/* ID */
					"PersonT.ID",
					/* Value */
					"PersonT.FullName",
					/* Description */
					"The list below contains all the providers that are categorized as Mid-Level in the Provider Roles section.  Please select the provider that is assigned to this username.",
					/* Force Selection */
					true) ){
		mprPrescriber.bCancelled = FALSE;
	}else{
		mprPrescriber.bCancelled = TRUE;
	}
	
	mprPrescriber.nID = dlg.GetSelectedID();
	mprPrescriber.strFullName = AsString(dlg.GetSelectedDisplayValue());

	return mprPrescriber;
}

// (b.savon 2013-01-11 10:41) - PLID 54584 - Select Multi-midlevel for Nurse/Staff
MultiMidlevelPrescriberReturn CNexERxSetupDlg::SelectMultiMidlevelPrescriber(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	MultiMidlevelPrescriberReturn mprPrescriber;

	// Don't include deactivated
	CString strAdditionalWhere = m_lNexERxLicense.GetInactiveMidlevelPrescriberIDString();
	if( !strAdditionalWhere.IsEmpty() ){
		strAdditionalWhere.Format(" AND PersonT.ID NOT IN (%s)", strAdditionalWhere);
	}

	CMultiSelectDlg dlg(this, "NexERxMultiMidlevel");
	CString strSupervising;
	CString strMidlevel;
	GetNurseStaffMidlevelSupervisingIDs(pRow, strSupervising, strMidlevel);
	if( !strMidlevel.IsEmpty() ){
		strMidlevel.Replace(",", " ");
		dlg.PreSelect(strMidlevel);
	}
	if( IDOK == dlg.Open(
					/* FROM */
					"ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID", 
					/* WHERE */
					"NexERxProviderTypeID = 1 AND PersonT.Archived = 0" + strAdditionalWhere,
					/* ID */
					"PersonT.ID",
					/* Value */
					"PersonT.FullName",
					/* Description */
					"The list below contains all the providers that are categorized as Mid-Level in the Provider Roles section.  Please select all Midlevel providers that this user may prescribe for.",
					/* Force Selection */
					true) ){
		mprPrescriber.bCancelled = FALSE;
	}else{
		mprPrescriber.bCancelled = TRUE;
	}

	mprPrescriber.strIDs = dlg.GetMultiSelectIDString(",");
	mprPrescriber.strFullNames = dlg.GetMultiSelectString("; ");
	
	return mprPrescriber;
}

SupervisingReturn CNexERxSetupDlg::SelectSupervisors(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	SupervisingReturn srSupervisors;

	CMultiSelectDlg dlg(this, "NexERxSupervisorSetup");

	// Don't include deactivated
	CString strAdditionalWhere = m_lNexERxLicense.GetInactiveLicensedPrescriberIDString();
	if( !strAdditionalWhere.IsEmpty() ){
		strAdditionalWhere.Format(" AND PersonT.ID NOT IN (%s)", strAdditionalWhere);
	}

	EUserRoleTypes urtUserRole;
	GetUserRole(pRow, urtUserRole);
	
	switch( urtUserRole ){
		case urtMidlevelPrescriber:
			{
				// (b.savon 2013-01-11 10:30) - PLID 54583 - Format for Midlevel
				CArray<long, long> aryPreselect;
				if( GetSupervisingIDs(pRow, aryPreselect) ){
					dlg.PreSelect(aryPreselect);
				}
			}
			break;
		case urtNurseStaff:
			{
				// (b.savon 2013-01-11 10:45) - PLID 54584 - Format for Nurse/Staff
				CString strSupervising;
				CString strMidlevel;
				GetNurseStaffMidlevelSupervisingIDs(pRow, strSupervising, strMidlevel);
				if( !strSupervising.IsEmpty() ){
					strSupervising.Replace(",", " ");
					dlg.PreSelect(strSupervising);
				}
			}
	}

	if( IDOK == dlg.Open(
					/* FROM */
					"ProvidersT INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID", 
					/* WHERE */
					"NexERxProviderTypeID = 0 AND PersonT.Archived = 0" + strAdditionalWhere,
					/* ID */
					"PersonT.ID",
					/* Value */
					"PersonT.FullName",
					/* Description */
					"The list below contains all the providers that are categorized as a Licensed Prescriber in the Provider Roles section.  Please select the provider that this user can prescribe for.") ){
		srSupervisors.bCancelled = FALSE;
	}else{
		srSupervisors.bCancelled = TRUE;
	}

	srSupervisors.strIDs = dlg.GetMultiSelectIDString(",");
	srSupervisors.strNames = dlg.GetMultiSelectString("; ");

	return srSupervisors;
}

// (b.savon 2013-01-11 10:45) - PLID 54584 - Nurse/Staff Wizard
void CNexERxSetupDlg::NurseStaffWizard(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	// Wizard Steps
	//  1. Ask if the user selected will prescribe for a Midlevel.
	CString strMessage;
	CString strUserName;
	if( GetUserName(pRow, strUserName) ){
		strMessage.Format("Will %s prescribe for a Midlevel?", strUserName);
	}else{
		strMessage = "Will this user prescribe for a Midlevel?";
	}
	// If they are prescribing for a Midlevel, Allow them to select multiple Midlevels.
	MultiMidlevelPrescriberReturn mprMidlevel;
	if( IDYES == MessageBox((LPCTSTR)strMessage, "NexERx Setup", MB_YESNO | MB_ICONQUESTION) ){
		 mprMidlevel = SelectMultiMidlevelPrescriber(pRow);
	}else{
		mprMidlevel.bCancelled = FALSE;
	}

	// Either way, let them select supervisors.
	//	1. Choose the Supervisor
	SupervisingReturn srSupervisors = SelectSupervisors(pRow);

	// The caller should be responsible for passing in a valid row, but let's be safe.
	if( pRow ){
		PutPrescribers(pRow, srSupervisors, mprMidlevel);
	}
}

void CNexERxSetupDlg::LeftClickNxdlUserRoles(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try{
		// If we have a valid row
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			// Get the current user role type
			EUserRoleTypes urtCurRole = (EUserRoleTypes)VarLong(pRow->GetValue(urcUserType), (long)urtNone);
			switch(urtCurRole){
				case urtLicensedPrescriber:
					{
						// (b.savon 2013-01-11 10:18) - PLID 54581 - If this user has a Licensed Prescriber role and they
						// clicked the hyperlink, start the wizard
						if( nCol == urcLicensedPrescriber ){
							LicensedPrescriberReturn lprLicensed = SelectLicensedPrescriber();
							if( !lprLicensed.bCancelled ){
								ClearLicensedPrescriber(pRow);
								PutLicensedPrescriber(pRow, lprLicensed);
							}
						}
					}
					break;
				case urtMidlevelPrescriber:
					{
						// (b.savon 2013-01-11 10:30) - PLID 54583 - If this user has a Midlevel role  and they
						// clicked the hyperlink, start the wizard
						if( nCol == urcMidlevelPrescriber ){
							MidlevelPrescriberReturn mprMidlevel = SelectMidlevelPrescriber();
							if( !mprMidlevel.bCancelled ){
								ClearMidlevelPrescriber(pRow);
								PutMidlevelPrescriber(pRow, mprMidlevel);
							}
						}else if( nCol == urcSupervisor ){
							SupervisingReturn srSupervisors = SelectSupervisors(pRow);
							PutSupervisors(pRow, srSupervisors);
						}
					}
					break;
				case urtNurseStaff:
					{
						// (b.savon 2013-01-11 10:45) - PLID 54584 - If this user has a Nurse/Staff role and they
						// clicked the hyperlink, start the wizard
						if( nCol == urcPrescribers ){
							NurseStaffWizard(pRow);
						}
					}
					break;
			}
		}

	}NxCatchAll(__FUNCTION__);
}

void CNexERxSetupDlg::OnBnClickedOk()
{
	try{
		
		if( !SaveRoles() ){
			return;
		}

	CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

BOOL CNexERxSetupDlg::SaveRoles(BOOL bSilent /*= FALSE*/)
{
	try{
		//Run through the list creating a sql statment to save to data.
		CSqlFragment sql;
		BOOL bReturn = TRUE;

		// Save the user roles.  The provider roles are save realtime.
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserRole->GetFirstRow();
		while(pRow){
	
			// (b.savon 2013-01-11 10:20) - PLID 54578
			//Gather the user info
			UserCommitInfo uciCurrentUser = GetCurrentUserInfo(pRow);

			//1. Get Link queries
			CSqlFragment sqlFrag = GetLinkSql(uciCurrentUser);
			if( sqlFrag.IsEmpty() ){
				return FALSE;
			}else{
				sql += sqlFrag;
			}
			
			//Next, Please
			pRow = pRow->GetNextRow();	
		}

		//Just make one trip
		if( !sql.IsEmpty() ){
			ExecuteParamSql("{SQL}", sql);
			if( !bSilent ){
				MessageBox("NexERx user configuration successfully applied!\nPlease restart Practice for these changes to take effect.", 
					"NexERx User Configuration", MB_ICONINFORMATION);
			}
		}
	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

// (b.savon 2013-01-11 10:20) - PLID 54578 - User Info Gatherer
UserCommitInfo CNexERxSetupDlg::GetCurrentUserInfo(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	UserCommitInfo uciCurrentUser;
	// Get the user person id.  If we get back a bad response, set our commit to invalid so we don't add it to the sql statement
	(GetUserPersonID(pRow, uciCurrentUser.nUserPersonID) == FALSE) ? (uciCurrentUser.bInvalid = TRUE) : (uciCurrentUser.bInvalid = FALSE);
	// Get the user role.  The following switch statement will handle if the role is valid or not.  If we get some bad response,
	// set our commit to invalid so we don't add it to the sql statement.
	GetUserRole(pRow, uciCurrentUser.urtUserRole);
	GetUserNameA(pRow, uciCurrentUser.strUserName);
	switch( uciCurrentUser.urtUserRole ){
		case urtLicensedPrescriber:
			{
				// (b.savon 2013-01-11 10:21) - PLID 54581 - Get the Licensed Prescriber ID if this user is one.
				if( !GetLicensedPrescriberID(pRow, uciCurrentUser.nLicensedPrescriberID) ){
					uciCurrentUser.bInvalid = TRUE;
				}
			}
			break;
		case urtMidlevelPrescriber:
			{
				// (b.savon 2013-01-11 10:31) - PLID 54583 - Get the midlevel ID if this user one.
				if( !GetMidlevelPrescriberID(pRow, uciCurrentUser.nMidlevelID) || !GetSupervisingIDs(pRow, uciCurrentUser.arySupervising)){
					uciCurrentUser.bInvalid = TRUE;
				}
			}
			break;
		case urtNurseStaff:
			{
				// (b.savon 2013-01-11 10:46) - PLID 54584 - Get the Nurse/Staff ids
				if( !GetNurseStaffPrescriberIDs(pRow, uciCurrentUser.arySupervising, uciCurrentUser.aryMidlevel) ){
					uciCurrentUser.bInvalid = TRUE;
				}
			}
			break;
		// We still want to save here in case the user had a role but then that role was taken away.
		case urtNone:
			{
				
			}
			break;
		// Don't save if there is no user role defined or if we got an odd result back
		default:
			// We shouldn't get here unless we've defined a new role and failed to handle it here.
			ASSERT(FALSE); 
			uciCurrentUser.bInvalid = TRUE;
			break;
	}

	return uciCurrentUser;
}

// (b.savon 2013-01-11 10:21) - PLID 54578
BOOL CNexERxSetupDlg::GetUserPersonID(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nUserPersonID)
{
	return ((nUserPersonID = VarLong(pRow->GetValue(urcID), -1)) != -1);
}

// (b.savon 2013-01-11 10:21) - PLID 54578
BOOL CNexERxSetupDlg::GetUserRole(NXDATALIST2Lib::IRowSettingsPtr pRow, EUserRoleTypes &urtUserRole)
{
	return ((urtUserRole = (EUserRoleTypes)VarLong(pRow->GetValue(urcUserType), urtNone)) != urtNone);
}
// (b.savon 2013-01-11 10:22) - PLID 54578
BOOL CNexERxSetupDlg::GetUserName(NXDATALIST2Lib::IRowSettingsPtr pRow, CString &strUserName)
{
	return (!(strUserName = VarString(pRow->GetValue(urcUserName), "")).IsEmpty());
}

// (b.savon 2013-01-11 10:21) - PLID 54581
BOOL CNexERxSetupDlg::GetLicensedPrescriberID(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nLicensedPrescriberID)
{
	return ((nLicensedPrescriberID = VarLong(pRow->GetValue(urcLicensedPrescriberID), -1)) != -1);
}

// (b.savon 2013-01-11 10:31) - PLID 54583
BOOL CNexERxSetupDlg::GetMidlevelPrescriberID(NXDATALIST2Lib::IRowSettingsPtr pRow, long &nMidlevelID)
{
	return ((nMidlevelID = VarLong(pRow->GetValue(urcMidlevelPrescriberID), -1)) != -1);
}

// (b.savon 2013-01-11 10:32) - PLID 54583
BOOL CNexERxSetupDlg::GetSupervisingIDs(NXDATALIST2Lib::IRowSettingsPtr pRow, CSimpleArray<long> &arySupervising)
{
	CArray<long, long> arySplit;
	GetSupervisingIDs(pRow, arySplit);
	for( int idx = 0; idx < arySplit.GetCount(); idx++ ){
		arySupervising.Add(arySplit.GetAt(idx));
	}

	if( arySupervising.GetSize() > 0 ){
		return arySupervising[0] != -1;
	}

	return FALSE;
}

// (b.savon 2013-01-11 10:32) - PLID 54583
BOOL CNexERxSetupDlg::GetSupervisingIDs(NXDATALIST2Lib::IRowSettingsPtr pRow, CArray<long, long> &arySupervising)
{
	if( pRow ){
		StringAsArray((LPCTSTR)VarString(pRow->GetValue(urcSupervisorIDs), "-1"), arySupervising);
	}

	if( arySupervising.GetCount() > 0 ){
		return arySupervising[0] != -1;
	}

	return FALSE;
}

BOOL CNexERxSetupDlg::GetSupervisingNames(NXDATALIST2Lib::IRowSettingsPtr pRow, CArray<CString, LPCTSTR> &arySupervising)
{
	if( pRow ){
		SplitNames(VarString(pRow->GetValue(urcSupervisor)), arySupervising, ";"); 
	}

	if( arySupervising.GetCount() > 0 ){
		return TRUE;
	}

	return FALSE;
}

// (b.savon 2013-01-11 10:47) - PLID 54584
BOOL CNexERxSetupDlg::GetNurseStaffPrescriberIDs(NXDATALIST2Lib::IRowSettingsPtr pRow, CSimpleArray<long> &arySupervising, CSimpleArray<long> &aryMidlevel)
{
	// Split and add supervising/midlevel to array.
	CArray<long, long> arySupervisingSplit;
	CArray<long, long> aryMidlevelSplit;
	GetNurseStaffPrescriberIDs(pRow, arySupervisingSplit, aryMidlevelSplit);
	
	for( int idx = 0; idx < arySupervisingSplit.GetCount(); idx++ ){
		arySupervising.Add(arySupervisingSplit.GetAt(idx));
	}
	for( int idx = 0; idx < aryMidlevelSplit.GetCount(); idx++ ){
		aryMidlevel.Add(aryMidlevelSplit.GetAt(idx));
	}

	if( arySupervising.GetSize() > 0 || aryMidlevel.GetSize() > 0 ){
		return TRUE;
	}

	return FALSE;
}

BOOL CNexERxSetupDlg::GetNurseStaffPrescriberIDs(NXDATALIST2Lib::IRowSettingsPtr pRow, CArray<long, long> &arySupervising, CArray<long, long> &aryMidlevel)
{
	CString strSupervising;
	CString strMidlevel;
	GetNurseStaffMidlevelSupervisingIDs(pRow, strSupervising, strMidlevel);

	// Split and add supervising to array.
	if( !strSupervising.IsEmpty() ){
		StringAsArray((LPCTSTR)strSupervising, arySupervising);
	}

	// Split and add midlevel to array.
	if( !strMidlevel.IsEmpty() ){
		StringAsArray((LPCTSTR)strMidlevel, aryMidlevel);
	}

	if( arySupervising.GetSize() > 0 || aryMidlevel.GetSize() > 0 ){
		return TRUE;
	}

	return FALSE;
}

BOOL CNexERxSetupDlg::GetNurseStaffPrescriberNames(NXDATALIST2Lib::IRowSettingsPtr pRow, CArray<CString, LPCTSTR> &arySupervisingNames, CArray<CString, LPCTSTR> &aryMidlevelNames)
{
	CString strSupervising;
	CString strMidlevel;
	GetNurseStaffMidlevelSupervisingNames(pRow, strSupervising, strMidlevel);

	// Split and add supervising to array.
	if( !strSupervising.IsEmpty() ){
		SplitNames(strSupervising, arySupervisingNames, ";");
	}

	// Split and add midlevel to array.
	if( !strMidlevel.IsEmpty() ){
		SplitNames(strMidlevel, aryMidlevelNames, ";");
	}

	if( arySupervisingNames.GetSize() > 0 || aryMidlevelNames.GetSize() > 0 ){
		return TRUE;
	}

	return FALSE;
}

// (b.savon 2013-01-11 10:58) - PLID 54584
void CNexERxSetupDlg::GetNurseStaffMidlevelSupervisingIDs(NXDATALIST2Lib::IRowSettingsPtr pRow, CString &strSupervising, CString &strMidlevel)
{
	GetNurseStaffMidlevelIdentifiers(VarString(pRow->GetValue(urcPrescriberIDs), "-1"), strSupervising, strMidlevel, nsiID);
}

// (b.savon 2013-01-11 10:58) - PLID 54584
void CNexERxSetupDlg::GetNurseStaffMidlevelSupervisingNames(NXDATALIST2Lib::IRowSettingsPtr pRow, CString &strSupervising, CString &strMidlevel)
{
	GetNurseStaffMidlevelIdentifiers(VarString(pRow->GetValue(urcPrescribers), "-1"), strSupervising, strMidlevel, nsiName);
}

CSqlFragment CNexERxSetupDlg::GetLinkSql(UserCommitInfo uciCurrentUser)
{
	CSqlFragment sqlFrag = CSqlFragment();
	// If the user commit is invalid, which means there is a red cell or cells then be sure to set this user
	// to a UserRole of None and save.
	if( uciCurrentUser.bInvalid ){
		CString strUser = uciCurrentUser.strUserName.IsEmpty() ? "A user" : uciCurrentUser.strUserName;
		CString strMessageText = strUser + " holds a license " +
								 "but has an invalid configuration.  Once a user's license has been deactivated, " +
								 "it can not be reactivated! Please deactivate the license only if the user will no longer " +
								 "be sending electronic prescriptions.\r\n\r\nIf you choose YES, the user's license " +
								 "will be deactivated.  If you choose NO, you will have the opportunity " +
								 "to configure the user correctly.\r\n\r\nAre you sure you would like to deactivate this user's license?";
		if( m_lNexERxLicense.IsUserLicensedNexERxNurseStaff(uciCurrentUser.nUserPersonID) ){
			if( IDYES == MessageBox(strMessageText, "NexTech Practice", MB_ICONEXCLAMATION | MB_YESNO) ){
				m_lNexERxLicense.DeactivateNurseStaff(uciCurrentUser.nUserPersonID);
			}else{
				return sqlFrag;
			}
		}
		uciCurrentUser.urtUserRole = urtNone;
	}

	sqlFrag += CSqlFragment("DELETE NexSupPT FROM NexERxSupervisingProviderT AS NexSupPT \r\n"
							"	INNER JOIN UsersT ON NexSupPT.PersonID = UsersT.NexERxProviderID \r\n"
							"	WHERE UsersT.PersonID = {INT} AND NexSupPT.UserID = {INT} \r\n"
							"DELETE NexNurPT FROM NexERxNurseStaffPrescriberT AS NexNurPT \r\n"
							"	INNER JOIN UsersT ON NexNurPT.UserID = UsersT.PersonID \r\n"
							"	WHERE UsersT.PersonID = {INT} AND NexNurPT.UserID = {INT} \r\n", 
							uciCurrentUser.nUserPersonID, uciCurrentUser.nUserPersonID, 
							uciCurrentUser.nUserPersonID, uciCurrentUser.nUserPersonID);

	switch( uciCurrentUser.urtUserRole ){
		case urtLicensedPrescriber:
			{
				// (b.savon 2013-01-11 10:23) - PLID 54581
				//No link, the user IS the licensed prescriber and is tied to the ProviderT given that their
				//user role is urtLicensedPrescriber. We are going to set the NexERxProviderID to the LicensedPrescriberID
				sqlFrag += CSqlFragment("UPDATE UsersT SET NexERxProviderID = {INT}, NexERxUserTypeID = {INT} WHERE PersonID = {INT} \r\n",
										uciCurrentUser.nLicensedPrescriberID, (long)uciCurrentUser.urtUserRole, uciCurrentUser.nUserPersonID);
			}
			break;
		case urtMidlevelPrescriber:
			{
				// (b.savon 2013-01-11 10:32) - PLID 54583
				//Midlevel's are tied to ProviderT already given that their user role is urtMidlevelPrescriber.  But,
				//Midlevel's also need to have a Supervising Prescriber (which is the Physician).  In most states,
				//they require Midlevel's send their Licensed prescriber.  For simplicity, we are going to make all
				//Midlevel's send their Licensed Prescriber because they can't prescribe without them in actuality.
				//Midlevel's cannot be in practice without a physician above them.  We are going to insert the links
				//into NexERxSupervisingProviderT.
				sqlFrag += CSqlFragment("UPDATE UsersT SET NexERxProviderID = {INT}, NexERxUserTypeID = {INT} WHERE PersonID = {INT} \r\n",
										uciCurrentUser.nMidlevelID, (long)uciCurrentUser.urtUserRole, uciCurrentUser.nUserPersonID);
				//Then, we set the links
				for( int idx = 0; idx < uciCurrentUser.arySupervising.GetSize(); idx++ ){
					sqlFrag += CSqlFragment("INSERT INTO NexERxSupervisingProviderT (PersonID, SupervisingPersonID, UserID) \r\n"
											"VALUES({INT}, {INT}, {INT}) \r\n", 
											uciCurrentUser.nMidlevelID, uciCurrentUser.arySupervising[idx], uciCurrentUser.nUserPersonID);
				}
			}
			break;
		case urtNurseStaff:
			{
				// (b.savon 2013-01-11 10:58) - PLID 54584
				sqlFrag += CSqlFragment("UPDATE UsersT SET NexERxProviderID = NULL, NexERxUserTypeID = {INT} WHERE PersonID = {INT} \r\n",
										(long)uciCurrentUser.urtUserRole, uciCurrentUser.nUserPersonID);

				//Then, we set the links
				// First, the supervisors
				for( int idx = 0; idx < uciCurrentUser.arySupervising.GetSize(); idx++ ){
					sqlFrag += CSqlFragment("INSERT INTO NexERxNurseStaffPrescriberT (NexERxUserType, NurseStaffPrescriberID, UserID) \r\n"
											"VALUES({INT}, {INT}, {INT}) \r\n", 
											(long)urtLicensedPrescriber, uciCurrentUser.arySupervising[idx], uciCurrentUser.nUserPersonID);
				}

				// Then, the midlevels
				for( int idx = 0; idx < uciCurrentUser.aryMidlevel.GetSize(); idx++ ){
					sqlFrag += CSqlFragment("INSERT INTO NexERxNurseStaffPrescriberT (NexERxUserType, NurseStaffPrescriberID, UserID) \r\n"
											"VALUES({INT}, {INT}, {INT}) \r\n", 
											(long)urtMidlevelPrescriber, uciCurrentUser.aryMidlevel[idx], uciCurrentUser.nUserPersonID);
				}
			}
			break;
		case urtNone:
			{
				//As discussed up in this code tree branch, we want to clear User links and roles in the case where the User had a role,
				//and it was revoked.
				sqlFrag += CSqlFragment("UPDATE UsersT SET NexERxProviderID = NULL, NexERxUserTypeID = {INT} WHERE PersonID = {INT} \r\n",
										(long)uciCurrentUser.urtUserRole, uciCurrentUser.nUserPersonID);
			}
			break;
		default:
			//Do nothing
			break;
	}

	return sqlFrag;
}

void CNexERxSetupDlg::SaveUserRole(NXDATALIST2Lib::IRowSettingsPtr pRow, EUserRoleTypes urtCurRole)
{
	//Clear the cells
	ClearLicensedPrescriber(pRow);
	ClearMidlevelPrescriber(pRow);
	ClearSupervisor(pRow);
	ClearPrescriber(pRow);
	//Wizardize New User Role Type Selections
	//Only if our new role isn't urtNone
	switch(urtCurRole){
		case urtLicensedPrescriber:
		{
			// (b.savon 2013-01-11 10:16) - PLID 54581 - Licensed Prescriber Wizard
			LicensedPrescriberReturn lprLicensed = LicensedPrescriberWizard();
			PutLicensedPrescriber(pRow, lprLicensed);
		}
		break;
		case urtMidlevelPrescriber:
			// (b.savon 2013-01-11 10:28) - PLID 54583 - Midlevel Wizard
			MidlevelPrescriberWizard(pRow);
			break;
		case urtNurseStaff:
			// (b.savon 2013-01-11 10:35) - PLID 54584 - Nurse/Staff Wizard
			NurseStaffWizard(pRow);
			break;
		case urtNone:
			pRow->PutValue(urcUserType, (long)urtCurRole);
			break;
	}
	//Apply the properties
	SetUserRolesCellProperties();
}

void CNexERxSetupDlg::EditingStartingNxdlProviderRole(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try{
		//If the row is valid
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			// If were in the provider type column
			if( nCol == prcProviderRoleID ){
				// If the value changed
				EProviderRoleTypes prtOldRole = (EProviderRoleTypes)VarLong(pRow->GetValue(prcProviderRoleID), (long)prtNone);
				long nProviderID = VarLong(pRow->GetValue(prcPersonID));
				if( IsProviderOrUserUsingLicense(nProviderID) ){
					MessageBox("This provider is using a license and their role cannot be changed unless you deactivate their license.  Once a provider's license has been deactivated, it can not be reactivated! Please use this feature only if the provider will no longer be sending electronic prescriptions.\r\n\r\nIf you have any questions or need assistance, please call Nextech Support.  If you are sure you would like to do this, click the 'Configure NexERx Licenses' button below to deactivate this provider's license.",
							   "NexERx Provider Licensed!", MB_ICONINFORMATION);
					*pbContinue = FALSE;
					return;
				}
				if( IsProviderOrUserDeactivated(nProviderID) ){
					MessageBox("This provider's license is deactivated and can no long be assigned a prescribing role.  If you have any questions or need assistance, please call Nextech Support.",
							   "NexERx Provider Deactivated!", MB_ICONINFORMATION);
					*pbContinue = FALSE;
					return;
				}
				if( prtOldRole != prtNone ){
					if( IsProviderAssignedToUsers(nProviderID) ){
						CString strMessage = VarString(pRow->GetValue(prcProviderName), "This provider") + " is assigned to users below. " +
						"If you answer 'Yes', the prescriber will be removed from any user that it was configured for." +
						"  Are you sure you would like to continue?";
						if( IDNO == MessageBox(strMessage, "NexERx Setup", MB_ICONEXCLAMATION | MB_YESNO)){
							*pbContinue = FALSE;
							return;
						}
					}
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

BOOL CNexERxSetupDlg::IsProviderAssignedToUsers(long nProviderID)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserRole->GetFirstRow();
	BOOL bFound = FALSE;
	m_arynUserIDsToUpdate.RemoveAll(); //Clear the array to start fresh
	while(pRow){
		EUserRoleTypes urtUserRole = urtNone;
		GetUserRole(pRow, urtUserRole);
		long nUserID = -1;
		GetUserPersonID(pRow, nUserID);

		switch( urtUserRole ){
			case urtLicensedPrescriber:
				{
					long nLicensedPrescriber = -1;
					GetLicensedPrescriberID(pRow, nLicensedPrescriber);
					if (CheckLicensedPrescriberProviderID(nLicensedPrescriber, nProviderID)){
						SaveUserID(nUserID);
						bFound = TRUE;
					}
				}
				break;
			case urtMidlevelPrescriber:
				{
					long nMidlevelPrescriber = -1;
					CSimpleArray<long> arynSupervisorIDs;
					GetMidlevelPrescriberID(pRow, nMidlevelPrescriber);
					GetSupervisingIDs(pRow, arynSupervisorIDs);
					if(CheckMidlevelPrescriberProviderID(nMidlevelPrescriber, nProviderID)){
						SaveUserID(nUserID);
						bFound = TRUE;
					}
					if(CheckSupervisingProviderID(arynSupervisorIDs, nProviderID)){
						SaveUserID(nUserID);
						bFound = TRUE;
					}
				}
				break;
			case urtNurseStaff:
				{
					CSimpleArray<long> arynSupervisorIDs;
					CSimpleArray<long> arynMidlevelIDs;
					GetNurseStaffPrescriberIDs(pRow, arynSupervisorIDs, arynMidlevelIDs);
					if(CheckSupervisingProviderID(arynSupervisorIDs, nProviderID)){
						SaveUserID(nUserID);
						bFound = TRUE;
					}
					if(CheckMultiMidlevelProviderID(arynMidlevelIDs, nProviderID)){
						SaveUserID(nUserID);
						bFound = TRUE;
					}
				}
				break;
			case urtNone:
				{

				}
				break;
		}

		pRow = pRow->GetNextRow();
	}
	return bFound;
}

BOOL CNexERxSetupDlg::IsEqual(long &lhs, long &rhs)
{
	return lhs == rhs;
}

BOOL CNexERxSetupDlg::IsFound(CSimpleArray<long> &arynIDs, long &nKey)
{
	BOOL bFound = FALSE;
	foreach(long nVal, arynIDs){
		if( nVal == nKey )
			return TRUE;
	}

	return FALSE;
}

void CNexERxSetupDlg::SaveUserID(long nUserID)
{
	m_arynUserIDsToUpdate.Add(nUserID);
}

BOOL CNexERxSetupDlg::CheckLicensedPrescriberProviderID(long &nLicensedPrescriberID, long &nProviderID)
{
	return IsEqual(nLicensedPrescriberID, nProviderID);
}

BOOL CNexERxSetupDlg::CheckMidlevelPrescriberProviderID(long &nMidlevelPrescriberID, long &nProviderID)
{
	return IsEqual(nMidlevelPrescriberID, nProviderID);
}

BOOL CNexERxSetupDlg::CheckSupervisingProviderID(CSimpleArray<long> &arynIDs, long &nKey)
{
	return IsFound(arynIDs, nKey);
}

BOOL CNexERxSetupDlg::CheckMultiMidlevelProviderID(CSimpleArray<long> &arynIDs, long &nKey)
{
	return IsFound(arynIDs, nKey);
}

void CNexERxSetupDlg::RemoveSupervisingProvider(CArray<long, long> &arynSupervisingIDs, CArray<CString, LPCTSTR> &arysSupervisingNames, long nProviderID)
{
	for( int idx = 0; idx < arynSupervisingIDs.GetCount(); idx++ ){
		if( arynSupervisingIDs.GetAt(idx) == nProviderID ){
			arynSupervisingIDs.RemoveAt(idx);
			arysSupervisingNames.RemoveAt(idx);
			return;
		}
	}
}

void CNexERxSetupDlg::ConstructDelimetedString(CArray<long, long> &arynIDs, CString &strIDs, const CString &strDelim)
{
	CString strTemp;
	for(int idx = 0; idx < arynIDs.GetCount(); idx++){
		strTemp.Format("%li", arynIDs.GetAt(idx));
		strIDs += strTemp + strDelim;
	}

	if (strIDs.GetLength() > 0) //either way strip the last char
		strIDs = strIDs.Left( strIDs.GetLength() - strDelim.GetLength() );
}

void CNexERxSetupDlg::ConstructDelimetedString(CArray<CString, LPCTSTR> &arynNames,CString &strNames, const CString &strDelim)
{
	for(int idx = 0; idx < arynNames.GetCount(); idx++){
		strNames += arynNames.GetAt(idx) + strDelim;
	}

	if (strNames.GetLength() > 0) //either way strip the last char
		strNames = strNames.Left( strNames.GetLength() - strDelim.GetLength() );
}

// (b.savon 2013-01-31 14:17) - PLID 54964
void CNexERxSetupDlg::OnBnClickedBtnConfigureLicenses()
{
	try{
		
		// First, save the roles so the license dialog is realtime
		if( !SaveRoles(TRUE) ){
			return;
		}

		CNexERxConfigureLicenses dlg(this, this);
		dlg.DoModal();

		EnforceDeactivatedLicenses();

	}NxCatchAll(__FUNCTION__);
}

void CNexERxSetupDlg::OnClose()
{
	try{
	
		if( !SaveRoles() ){
			return;
		}

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxSetupDlg::OnCancel()
{
	try{

		//Do Nothing. Disallow them from hitting ESC to close

	}NxCatchAll(__FUNCTION__);
}

long CNexERxSetupDlg::GetCountProviderRole(EProviderRoleTypes prtProviderRole)
{
	long nCount = 0;
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlProviderRole->GetFirstRow();
		
		while(pRow){
			if( VarLong(pRow->GetValue(prcProviderRoleID), prtNone) == prtProviderRole ){
				++nCount;
			}

			pRow = pRow->GetNextRow();
		}
	}NxCatchAll(__FUNCTION__);
	return nCount;
}

long CNexERxSetupDlg::GetCountUserRole(EUserRoleTypes urtUserRole)
{
	long nCount = 0;
	try{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserRole->GetFirstRow();
		
		while(pRow){
			if( VarLong(pRow->GetValue(urcUserType), urtNone) == urtNurseStaff ){
				++nCount;
			}

			pRow = pRow->GetNextRow();
		}
	}NxCatchAll(__FUNCTION__);
	return nCount;
}

// (b.savon 2013-04-01 17:25) - PLID 54578
BOOL CNexERxSetupDlg::IsValidPrescriberSelection(long nProviderID, EUserRoleColumns urcProviderIDColumn)
{
	try{
		// Users are tied directly to Licensed Prescriber and Midlevel Prescriber Providers.
		// There may not be 2 users that have a is-the relationship to a configured Provider.
		return m_nxdlUserRole->FindByColumn(urcProviderIDColumn, nProviderID, NULL, VARIANT_FALSE) ? FALSE : TRUE;
	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

BOOL CNexERxSetupDlg::IsProviderOrUserUsingLicense(long nPersonID)
{
	return  m_lNexERxLicense.IsUserLicensedNexERxLicensedPrescriber(nPersonID) ||
			m_lNexERxLicense.IsUserLicensedNexERxMidlevelPrescriber(nPersonID) ||
			m_lNexERxLicense.IsUserLicensedNexERxNurseStaff(nPersonID);
}

BOOL CNexERxSetupDlg::IsProviderOrUserDeactivated(long nPersonID)
{
	CDWordArray dwaInactiveLicensedPrescriberIDs;
	CDWordArray dwaInactiveMidlevelPrescriberIDs;
	CDWordArray dwaInactiveNurseStaffIDs;
	m_lNexERxLicense.GetInactiveLicensedPrescriberIDs(dwaInactiveLicensedPrescriberIDs);
	m_lNexERxLicense.GetInactiveMidlevelPrescriberIDs(dwaInactiveMidlevelPrescriberIDs);
	m_lNexERxLicense.GetInactiveNurseStaffIDs(dwaInactiveNurseStaffIDs);

	//Deactivated
	foreach(DWORD dwID, dwaInactiveLicensedPrescriberIDs){
		if( (long)dwID == nPersonID ){
			return TRUE;
		}
	}

	foreach(DWORD dwID, dwaInactiveMidlevelPrescriberIDs){
		if( (long)dwID == nPersonID ){
			return TRUE;
		}
	}

	foreach(DWORD dwID, dwaInactiveNurseStaffIDs){
		if( (long)dwID == nPersonID ){
			return TRUE;
		}
	}

	return FALSE;
}

void CNexERxSetupDlg::EnforceDeactivatedLicenses()
{
	CDWordArray dwaInactiveLicensedPrescriberIDs;
	CDWordArray dwaInactiveMidlevelPrescriberIDs;
	CDWordArray dwaInactiveNurseStaffIDs;
	m_lNexERxLicense.GetInactiveLicensedPrescriberIDs(dwaInactiveLicensedPrescriberIDs);
	m_lNexERxLicense.GetInactiveMidlevelPrescriberIDs(dwaInactiveMidlevelPrescriberIDs);
	m_lNexERxLicense.GetInactiveNurseStaffIDs(dwaInactiveNurseStaffIDs);

	// Do the LicensedPrescribers
	foreach(DWORD dwID, dwaInactiveLicensedPrescriberIDs){
		HandleDeactivatedPrescriber(dwID);
	}

	// Do the MidlevelPrescribers
	foreach(DWORD dwID, dwaInactiveMidlevelPrescriberIDs){
		HandleDeactivatedPrescriber(dwID);
	}

	// Do the NurseStaff
	NXDATALIST2Lib::IRowSettingsPtr pRow;
	foreach(DWORD dwID, dwaInactiveNurseStaffIDs){
		pRow = m_nxdlUserRole->FindByColumn(urcID, (long)dwID, NULL, VARIANT_FALSE);
		if( pRow ){
			SaveUserRole(pRow, urtNone);
		}
	}
}

void CNexERxSetupDlg::HandleDeactivatedPrescriber(DWORD dwProviderID)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlProviderRole->FindByColumn(prcPersonID, (long)dwProviderID, NULL, VARIANT_FALSE);
	if( pRow ){
		pRow->PutValue(prcProviderRoleID, prtNone);
		ExecuteParamSql("UPDATE ProvidersT SET NexERxProviderTypeID = {INT} WHERE PersonID = {INT}", (long)prtNone, (long)dwProviderID);
		if( IsProviderAssignedToUsers((long)dwProviderID) ){
			UnregisterPrescriber((long)dwProviderID);
		}
	}
}

void CNexERxSetupDlg::UnregisterPrescriber(long nProviderID)
{
	try{
		EUserRoleTypes urtUserRole = urtNone;
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		foreach(long nUserID, m_arynUserIDsToUpdate){
			pRow = m_nxdlUserRole->FindByColumn(urcID, nUserID, NULL, FALSE);
			if( pRow ){
				GetUserRole(pRow, urtUserRole);
				switch(urtUserRole){
					case urtLicensedPrescriber:
						{
							SaveUserRole(pRow, urtNone);
						}
						break;
					case urtMidlevelPrescriber:
						{
							long nMidlevelPrescriber = -1;
							GetMidlevelPrescriberID(pRow, nMidlevelPrescriber);
							if(CheckMidlevelPrescriberProviderID(nMidlevelPrescriber, nProviderID)){
								ClearMidlevelPrescriber(pRow);
								break;
							}
							//We know the provider id were looking for is a supervisor at this point.
							//Just remove it from the list

							//There is a list of steps we have to do here. 
							// 1. Get the IDs and Names and store in an array
							// 2. Remove the matching prescriber from the arrays
							// 3. Construct the delimeted lists from the new array
							// 4. Construct the object
							// 5. Put the new values back into the datalist

							//1.
							CArray<long, long> arynSupervisorIDs;
							CArray<CString, LPCTSTR> arysSupervisorNames;
							GetSupervisingIDs(pRow, arynSupervisorIDs);
							GetSupervisingNames(pRow, arysSupervisorNames);
							//2.
							RemoveSupervisingProvider(arynSupervisorIDs, arysSupervisorNames, nProviderID);
							//3.
							CString strNames;
							CString strIDs;
							ConstructDelimetedString(arynSupervisorIDs, strIDs, ",");
							ConstructDelimetedString(arysSupervisorNames, strNames, ";");
							//4.
							SupervisingReturn srSupervisors;
							srSupervisors.strIDs = strIDs;
							srSupervisors.strNames = strNames;
							srSupervisors.bCancelled = FALSE;
							//5.
							PutSupervisors(pRow, srSupervisors);
						}
						break;
					case urtNurseStaff:
						{
							//There is a list of steps we have to do here.
							// 1. Get the IDs and Names and store in an array
							// 2. Remove the matching prescriber from the arrays
							// 3. Construct the delimeted lists from the new arrays
							// 4. Construct the objects
							// 5. Put the new values back into the datalist

							//1.
							CArray<long, long> arynSupervisingIDs;
							CArray<long, long> arynMidlevelIDs;
							CArray<CString, LPCTSTR> arysSupervisingNames;
							CArray<CString, LPCTSTR> arysMidlevelNames;
							GetNurseStaffPrescriberIDs(pRow, arynSupervisingIDs, arynMidlevelIDs);
							GetNurseStaffPrescriberNames(pRow, arysSupervisingNames, arysMidlevelNames);
							//2.
							RemoveSupervisingProvider(arynSupervisingIDs, arysSupervisingNames, nProviderID);
							RemoveSupervisingProvider(arynMidlevelIDs, arysMidlevelNames, nProviderID);
							//3.
							CString strSupervisingNames;
							CString strMidlevelNames;
							CString strSupervisingIDs;
							CString strMidlevelIDs;
							ConstructDelimetedString(arynSupervisingIDs, strSupervisingIDs, ",");
							ConstructDelimetedString(arynMidlevelIDs, strMidlevelIDs, ",");
							ConstructDelimetedString(arysSupervisingNames, strSupervisingNames, ";");
							ConstructDelimetedString(arysMidlevelNames, strMidlevelNames, ";");
							//4.
							MultiMidlevelPrescriberReturn mlMidlevel;
							mlMidlevel.bCancelled = FALSE;
							mlMidlevel.strFullNames = strMidlevelNames;
							mlMidlevel.strIDs = strMidlevelIDs;
							SupervisingReturn srSupervisor;
							srSupervisor.bCancelled = FALSE;
							srSupervisor.strIDs = strSupervisingIDs;
							srSupervisor.strNames = strSupervisingNames;
							//5.
							PutPrescribers(pRow, srSupervisor, mlMidlevel, TRUE);
						}
						break;
				}
			}
		}
		//After we apply changes, always set the cell properties
		SetUserRolesCellProperties();
	}NxCatchAll(__FUNCTION__);
}

void CNexERxSetupDlg::EditingStartingNxdlUserRoles(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue)
{
	try{
		//If the row is valid
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if( pRow ){
			// If were in the user type column
			if( nCol == urcUserType ){
				EUserRoleTypes urtRole = (EUserRoleTypes)VarLong(pRow->GetValue(urcUserType), (long)urtNone);
				long nUserID = VarLong(pRow->GetValue(urcID));
				if( IsProviderOrUserUsingLicense(nUserID) ){
					MessageBox("This user is using a license and their role cannot be changed unless you deactivate their license.  Once a user's Nurse/Staff license has been deactivated, it can not be reactivated! Please use this feature only if the user will no longer be sending electronic prescriptions.\r\n\r\nIf you have any questions or need assistance, please call Nextech Support.  If you are sure you would like to do this, click the 'Configure NexERx Licenses' button below to deactivate this user's license.",
							   "NexERx User Licensed!", MB_ICONINFORMATION);
					*pbContinue = FALSE;
					return;
				}
				if( IsProviderOrUserDeactivated(nUserID) ){
					MessageBox("This user's license is deactivated and can no long be assigned a prescribing role.  If you have any questions or need assistance, please call Nextech Support.",
							   "NexERx User Deactivated!", MB_ICONINFORMATION);
					*pbContinue = FALSE;
					return;
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

UserCommitInfo CNexERxSetupDlg::GetUserRoleInfo(long nPersonID)
{
	try{

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_nxdlUserRole->FindByColumn(urcID, nPersonID, NULL, FALSE);

		if( pRow ){
			return GetCurrentUserInfo(pRow);
		}

	}NxCatchAll(__FUNCTION__);

	return UserCommitInfo();
}