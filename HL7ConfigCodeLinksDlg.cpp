// HL7ConfigCodeLinksDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialRc.h"
#include "HL7ConfigCodeLinksDlg.h"
#include "AuditTrail.h"
#include "HL7ParseUtils.h"
#include "SharedInsuranceUtils.h"
#include "HL7Utils.h"
#include "HL7CodeLinkImportDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//TES 5/24/2010 - PLID 38865 - Changed CHL7ConfigLocationsDlg to CHL7ConfigCodeLinksDlg.  You now must pass in an HL7CodeLink_RecordType
// enum value, to specify which set of codes you're modifying.  I didn't bother putting PLID comments everywhere.
/////////////////////////////////////////////////////////////////////////////
// CHL7ConfigCodeLinksDlg dialog

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum ListColumns {

	lcThirdPartyCode = 0,
	lcPracticeID,
	//TES 9/16/2010 - PLID 40518 - We now track the old values in the datalist.
	lcOldThirdPartyCode,
	lcOldPracticeID,
};

CHL7ConfigCodeLinksDlg::CHL7ConfigCodeLinksDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7ConfigCodeLinksDlg::IDD, pParent)
{
	m_nHL7GroupID = -1;
	m_hclrtType = hclrtInvalid;
	//{{AFX_DATA_INIT(CHL7ConfigCodeLinksDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CHL7ConfigCodeLinksDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHL7ConfigCodeLinksDlg)
	DDX_Control(pDX, IDC_IMPORT_HL7_CODE_MAP, m_btnImport);
	DDX_Control(pDX, IDC_ADD_HL7_CODE_MAP, m_btnAdd);
	DDX_Control(pDX, IDC_REMOVE_HL7_CODE_MAP, m_btnRemove);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHL7ConfigCodeLinksDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHL7ConfigCodeLinksDlg)
	ON_BN_CLICKED(IDC_ADD_HL7_CODE_MAP, OnAddHl7CodeMap)
	ON_BN_CLICKED(IDC_REMOVE_HL7_CODE_MAP, OnRemoveHl7CodeMap)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_IMPORT_HL7_CODE_MAP, &CHL7ConfigCodeLinksDlg::OnBnClickedImportHl7CodeMap)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHL7ConfigCodeLinksDlg message handlers

BOOL CHL7ConfigCodeLinksDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// (j.jones 2008-05-08 09:38) - PLID 29953 - added nxiconbuttons for modernization
		// (r.gonet 09/27/2011) - PLID 45719 - Added an import button
		m_btnImport.AutoSet(NXB_IMPORTBOX);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		SetWindowText("Configure HL7 Mapping For " + GetHL7CodeLinkRecordName(m_hclrtType) + " Codes");

		m_pCodeMapList = BindNxDataList2Ctrl(this, IDC_HL7_CODE_MAP, GetRemoteData(), false);

		IColumnSettingsPtr pCol = m_pCodeMapList->GetColumn(lcPracticeID);
		switch(m_hclrtType) {
			case hclrtLocation:
				// (j.dinatale 2011-10-21 09:28) - PLID 46048 - Dont allow linking to the Practice Default, it causes lots of problems
				pCol->PutComboSource(_bstr_t(FormatString("SELECT ID, Name FROM LocationsT "
					"WHERE (Active = 1 AND (TypeID = 1 OR TypeID = 2)) OR ID IN (SELECT PracticeID FROM HL7CodeLinkT WHERE Type = %i) "
					//"UNION SELECT -1, '{Use Practice Default}' "
					"ORDER BY Name", hclrtLocation)));
				break;
			//TES 5/26/2010 - PLID 38660 - Added hclrtLabResultFlag
			case hclrtLabResultFlag:
				pCol->PutComboSource(_bstr_t(FormatString("SELECT ID, Name FROM LabResultFlagsT "
					"WHERE Inactive = 0 OR ID IN (SELECT PracticeID FROM HL7CodeLinkT WHERE Type = %i) "
					"ORDER BY Name", hclrtLabResultFlag)));
				break;
			//TES 6/1/2010 - PLID 38066 - Added hclrtLabSideAndQual
			case hclrtLabSideAndQual:
				pCol->PutComboSource(_bstr_t(FormatString("SELECT -1 AS ID, '<Left>' AS Name "
					"UNION SELECT -2 AS ID , '<Right>' AS Name "
					"UNION SELECT ID, Name FROM AnatomyQualifiersT "
					"WHERE Inactive = 0 OR ID IN (SELECT PracticeID FROM HL7CodeLinkT WHERE Type = %i) "
					"ORDER BY Name", hclrtLabSideAndQual)));
				break;
			//TES 6/3/2010 - PLID 38776 - Added hclrtLabField
			case hclrtLabResultField:
				{
					CString strComboSource = "SELECT ";
					//TES 6/3/2010 - PLID 38776 - Loop through everything in the enum (hlrfFinalDiagnosisCode will always be first,
					// the unused enum hlrfInvalid will always be last).
					for(int i = hlrfFinalDiagnosisCode; i < hlrfInvalid; i++) {
						strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", i, GetLabResultFieldName((HL7LabResultField)i));
					}
					//TES 6/3/2010 - PLID 38776 - Remove the last " UNION SELECT "
					strComboSource = strComboSource.Left(strComboSource.GetLength()-14);
					pCol->PutComboSource(_bstr_t(strComboSource));
					break;
				}
			//TES 6/30/2010 - PLID 39320 - Added hclrtInsCo
			case hclrtInsCo:
				{
					pCol->PutComboSource(_bstr_t(FormatString("SELECT PersonID, Name "
						"FROM InsuranceCoT INNER JOIN PersonT ON InsuranceCoT.PersonID = PersonT.ID "
						"WHERE Archived = 0 OR PersonID IN (SELECT PracticeID FROM HL7CodeLinkT WHERE Type = %i) "
						"ORDER BY Name", hclrtInsCo)));

					// Show the import button
					m_btnImport.ShowWindow(SW_SHOW);
				}
				break;
			// (z.manning 2010-07-07 15:17) - PLID 39559 - Resources
			case hclrtResource:
				{
					pCol->PutComboSource(_bstr_t(FormatString(
						"SELECT ResourceT.ID, ResourceT.Item \r\n"
						"FROM ResourceT \r\n"
						"WHERE ResourceT.Inactive = 0 OR ResourceT.ID IN \r\n"
						"	(SELECT HL7CodeLinkT.PracticeID FROM HL7CodeLinkT WHERE HL7CodeLinkT.Type = %d) \r\n"
						, hclrtResource)));
				}
				break;
			//TES 7/13/2010 - PLID 39610 - Added hclrtRelationToPatient
			case hclrtRelationToPatient:
				{
					CString strComboSource = "SELECT ";
					//TES 7/13/2010 - PLID 39610 - Loop through everything in the enum (prtSelf will always be first,
					// the unused enum rtp_LastEnum will always be last).
					// (j.jones 2011-06-28 12:03) - PLID 40959 - we can no longer loop through an enum,
					// but fortunately the list is much shorter now, so just add them all individually
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpSelf, GetRelation(rtpSelf));
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpChild, GetRelation(rtpChild));
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpSpouse, GetRelation(rtpSpouse));
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpOther, GetRelation(rtpOther));
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpUnknown, GetRelation(rtpUnknown));
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpEmployee, GetRelation(rtpEmployee));
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpOrganDonor, GetRelation(rtpOrganDonor));
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpCadaverDonor, GetRelation(rtpCadaverDonor));
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rtpLifePartner, GetRelation(rtpLifePartner));

					//TES 7/13/2010 - PLID 39610 - Remove the last " UNION SELECT "
					strComboSource = strComboSource.Left(strComboSource.GetLength()-14);
					pCol->PutComboSource(_bstr_t(strComboSource));
				}
				break;
			// (d.singleton 2012-08-27 15:45) - PLID 52302 added hclrtInsuranceCategoryType
			case hclrtInsuranceCategoryType:
				{
					CString strComboSource = "SELECT ";
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rctMedical, "Medical");
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rctVision, "Vision");
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rctAuto, "Auto");
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rctWorkersComp, "Workers'' Comp.");
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rctDental, "Dental");
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rctStudy, "Study");
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rctLOP, "Letter of Protection");
					strComboSource += FormatString("%i AS ID, '%s' AS Name UNION SELECT ", rctLOA, "Letter of Agreement");

					strComboSource = strComboSource.Left(strComboSource.GetLength()-14);
					pCol->PutComboSource(_bstr_t(strComboSource));
				}
				break;
					
			// (d.thompson 2012-08-21) - PLID 52048 - Support ethnicity
			case hclrtEthnicity:
				{
					pCol->PutComboSource(_bstr_t(
						"SELECT EthnicityT.ID, EthnicityT.Name \r\n"
						"FROM EthnicityT \r\n"
						"ORDER BY EthnicityT.Name "));
				}
				break;

			// (d.thompson 2012-08-22) - PLID 52047 - Support race
			case hclrtRace:
				{
					pCol->PutComboSource(_bstr_t(
						"SELECT RaceT.ID, RaceT.Name \r\n"
						"FROM RaceT \r\n"
						"ORDER BY RaceT.Name "));
				}
				break;

			// (d.thompson 2012-08-23) - PLID 52049 - Support language
			case hclrtLanguage:
				{
					pCol->PutComboSource(_bstr_t(
						"SELECT LanguageT.ID, LanguageT.Name \r\n"
						"FROM LanguageT \r\n"
						"ORDER BY LanguageT.Name "));
				}
				break;
			// (r.farnworth 2015-01-20 13:39) - PLID 64624 - Add the ability to override Charge Codes when importing charges through HL7.
			case hclrtServiceCode:
				{
					pCol->PutComboSource(_bstr_t(
						"SELECT CPTCodeT.ID, \r\n"
						"CPTCodeT.Code + \r\n"
						"CASE WHEN (CPTCodeT.SubCode IS NOT NULL AND CPTCodeT.SubCode <> '') THEN ' ' + CPTCodeT.SubCode ELSE '' END  + \r\n"
						"' - ' + ServiceT.Name \r\n"
						"FROM CPTCodeT \r\n"
						"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID \r\n"
						"ORDER BY CPTCodeT.Code "));
				}
				break;
			// (b.eyers 2015-06-08) - PLID 66205 - support inventory items
			case hclrtInventory:
			{
				pCol->PutComboSource(_bstr_t(
					"SELECT ProductT.ID, ServiceT.Name FROM ProductT \r\n"
					"INNER JOIN ServiceT ON ProductT.ID = ServiceT.ID \r\n"
					"ORDER BY ServiceT.Name"));
			}
				break;
			default:
				AfxThrowNxException("Invalid type passed to CHL7ConfigCodeLinksDlg");
				break;
		}

		//TES 9/16/2010 - PLID 40518 - We now track the old values in the datalist, rather than maintaining member variables.  So, 
		// we just need to requery the list (after filtering on the given group and type).
		m_pCodeMapList->WhereClause = _bstr_t(FormatString("Type = %li AND HL7GroupID = %li", m_hclrtType, m_nHL7GroupID));
		m_pCodeMapList->Requery();

		GetDlgItem(IDC_ADD_HL7_CODE_MAP)->EnableWindow(TRUE);
		GetDlgItem(IDC_REMOVE_HL7_CODE_MAP)->EnableWindow(m_pCodeMapList->CurSel == NULL ? FALSE : TRUE);

		return TRUE;

	}NxCatchAll("Error in CHL7ConfigCodeLinksDlg::OnInitDialog()");

	//We failed to load, so abort!
	OnCancel();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHL7ConfigCodeLinksDlg::OnAddHl7CodeMap() 
{
	try {
		BOOL bDone = FALSE;
		while(!bDone) {
			CString strCode;
			CString strRecordName = GetHL7CodeLinkRecordName(m_hclrtType);
			//TES 6/30/2010 - PLID 39320 - Insurance companies are linked here, but are used in the export, not the import.  So, I re-worded
			// this message to not say "import"
			if(InputBoxLimited(this, "Enter the " + strRecordName + " code you wish to link: ", strCode, "", 255, false, false, NULL) == IDOK) {
				//TES 8/19/2010 - PLID 38899 - For result flags, we trim leading/trailing spaces when we import, thus our validation
				// needs to do the same.
				if(m_hclrtType == hclrtLabResultFlag) {
					strCode.TrimLeft();
					strCode.TrimRight();
				}
				//Did they enter a code?
				if(!strCode.IsEmpty()) {
					//Is it in the list already?

					//compare case-insensitively
					BOOL bFound = FALSE;
					//TES 1/3/2011 - PLID 40518 - Make sure we only warn them once.
					BOOL bWarned = FALSE;
					IRowSettingsPtr pRow = m_pCodeMapList->GetFirstRow();
					while(pRow != NULL && !bFound) {
						CString strCurCode = VarString(pRow->GetValue(0), "");
						if(strCurCode.CompareNoCase(strCode) == 0) {
							//TES 9/16/2010 - PLID 40518 - Insurance Companies allow multiple companies to be linked to the same code.
							if(m_hclrtType != hclrtInsCo) {
								bFound = TRUE;

								//Yup, let them know their mistake.
								MsgBox("The code you entered is already mapped to an existing " + strRecordName + " in Practice.  Please enter a new code, "
									"or if you wish to change the mapping for this code, click in the 'Practice Code' column to select "
									"a different " + strRecordName + ".");
							}
							else {
								//TES 9/16/2010 - PLID 40518 - While we allow this, it could potentially cause problems when importing.  So,
								// if they're set to import insurance information, give them a warning, though we still allow them to continue.
								//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
								if(GetHL7SettingBit(m_nHL7GroupID, "ImportInsurance")) {
									if(!bWarned && IDYES != MsgBox(MB_YESNO, "WARNING: The code you entered is already mapped to an existing Insurance Company in Practice.  "
										"If you map the same code to multiple Insurance Companies, any messages imported with that code may be linked with "
										"an unexpected Insurance Company.  Are you SURE you wish to continue?")) {
											bFound = TRUE;
									}
									else {
										//TES 1/3/2011 - PLID 40518 - They said Yes, so don't warn them again.
										bWarned = TRUE;
									}
								}
							}
						}

						pRow = pRow->GetNextRow();
					}
		
					// (b.eyers 2015-06-10) - PLID 66205 - when in mdi mode, inventory and charges can't share the same code
					if (!bFound && m_hclrtType == hclrtServiceCode) {
						//see if code exists in inventory
						_RecordsetPtr rsCode;
						rsCode = CreateParamRecordset("SELECT ThirdPartyCode FROM HL7CodeLinkT WHERE Type = {INT}", hclrtInventory);

						CString strName;
						while (!rsCode->eof && !bFound) {
							strName = AdoFldString(rsCode, "ThirdPartyCode");
							if (strName.CompareNoCase(strCode) == 0) {
								bFound = TRUE;
								MsgBox("The code you entered is already mapped to an existing Inventory Item in Practice.  Please enter a new code, "
									"or if you wish to change the mapping for this code, go to the Inventory Overrides and select "
									"a different Inventory Item. Inventory Overrides button is only visible when the IntelleChart Advanced Settings is enabled.");
							}
							rsCode->MoveNext();
						}
						rsCode->Close();
					}
					else if (!bFound && m_hclrtType == hclrtInventory) {
						//see if code exists in servicecode
						_RecordsetPtr rsCode;
						rsCode = CreateParamRecordset("SELECT ThirdPartyCode FROM HL7CodeLinkT WHERE Type = {INT}", hclrtServiceCode);

						CString strName;
						while (!rsCode->eof && !bFound) {
							strName = AdoFldString(rsCode, "ThirdPartyCode");
							if (strName.CompareNoCase(strCode) == 0) {
								bFound = TRUE;
								MsgBox("The code you entered is already mapped to an existing Service Code in Practice.  Please enter a new code, "
									"or if you wish to change the mapping for this code, go to the Charge Code Overrides and select "
									"a different Service Code.");
							}
							rsCode->MoveNext();
						}
						rsCode->Close();

					}

					if(!bFound) {
						//Nope.  Let's add it.
						IRowSettingsPtr pRow = m_pCodeMapList->GetNewRow();
						pRow->PutValue(lcThirdPartyCode, _bstr_t(strCode));
						// (j.dinatale 2011-10-21 12:26) - PLID 46048 - default to the current location, so that way we dont put
						//		bad location links in HL7CodeLinkT
						if(m_hclrtType == hclrtLocation){
							pRow->PutValue(lcPracticeID, (long)GetCurrentLocationID());
						}else{
							pRow->PutValue(lcPracticeID, g_cvarNull);
						}
						//TES 9/16/2010 - PLID 40518 - Fill in NULL for the old values.
						pRow->PutValue(lcOldThirdPartyCode, g_cvarNull);
						pRow->PutValue(lcOldPracticeID, g_cvarNull);
						m_pCodeMapList->AddRowAtEnd(pRow, NULL);
						m_pCodeMapList->CurSel = pRow;
						GetDlgItem(IDC_REMOVE_HL7_CODE_MAP)->EnableWindow(TRUE);
						m_pCodeMapList->StartEditing(pRow, lcPracticeID);
						//They've entered a valid code, we're done.
						bDone = TRUE;
					}
				}
			}
			else {
				//They cancelled, we're done.
				bDone = TRUE;
			}
		}
	}NxCatchAll("Error in CHL7ConfigCodeLinksDlg::OnAddHl7CodeMap()");
}

void CHL7ConfigCodeLinksDlg::OnRemoveHl7CodeMap() 
{
	try {
		if(m_pCodeMapList->CurSel != NULL) {
			IRowSettingsPtr pRow = m_pCodeMapList->CurSel;
			//TES 9/16/2010 - PLID 40518 - If this is not a new map, add it to our array of deleted maps.
			CString strOldCode = VarString(pRow->GetValue(lcOldThirdPartyCode),"");
			long nOldPracticeID = VarLong(pRow->GetValue(lcOldPracticeID),-1);
			if(!strOldCode.IsEmpty()) {
				CodeMap cm;
				cm.strCode = strOldCode;
				cm.nPracticeID = nOldPracticeID;
				m_arDeletedCodeMaps.Add(cm);
			}
			m_pCodeMapList->RemoveRow(m_pCodeMapList->CurSel);
		}
	}NxCatchAll("Error in CHL7ConfigCodeLinksDlg::OnRemoveHl7CodeMap()");
}

void CHL7ConfigCodeLinksDlg::OnOK() 
{
	long nAuditTransactionID = -1;

	try {

		// (j.jones 2015-11-16 12:44) - PLID 67491 - disallow new links with no practice ID
		{
			bool bAtLeastOneExistingRowNull = false;
			IRowSettingsPtr pRow = m_pCodeMapList->GetFirstRow();
			while (pRow) {
				CString strThirdPartyCode = VarString(pRow->GetValue(lcThirdPartyCode), "");
				long nPracticeID = VarLong(pRow->GetValue(lcPracticeID), -1);
				CString strOldCode = VarString(pRow->GetValue(lcOldThirdPartyCode), "");

				//TES 6/10/2010 - PLID 38066 - -1 is a valid PracticeID for hclrtLabSideAndQual.  Not the best decision I've ever made, 
				// but that ship has sailed.
				if (nPracticeID == -1 && m_hclrtType != hclrtLabSideAndQual) {
					if (strOldCode.IsEmpty()) {
						//this is a new link - it cannot be null
						MessageBox(FormatString("The HL7 code '%s' must be linked to a valid Practice code.", strThirdPartyCode), "Practice", MB_ICONEXCLAMATION | MB_OK);
						return;
					}
					else {
						//this is an existing link
						bAtLeastOneExistingRowNull = true;
					}
				}

				pRow = pRow->GetNextRow();
			}

			if (bAtLeastOneExistingRowNull) {
				if (IDYES == MessageBox("At least one HL7 code is not linked to a valid Practice code. All HL7 codes should be mapped to a Practice code, or otherwise be deleted if no Practice mapping exists.\n\n"
					"Do you wish to cancel saving and correct these entries now?", "Practice", MB_ICONEXCLAMATION | MB_YESNO)) {
					return;
				}
			}
		}
		
		//Let's save, all in one batch.
		CString strSql = BeginSqlBatch();

		AuditEventItems aeiAuditItem = (AuditEventItems)-1;
		switch(m_hclrtType) {
			case hclrtLocation:
				aeiAuditItem = aeiHL7LocationLink;
				break;
			//TES 5/26/2010 - PLID 38660 - Added hclrtLabResultFlag
			case hclrtLabResultFlag:
				aeiAuditItem = aeiHL7LabResultFlagLink;
				break;
			//TES 6/1/2010 - PLID 38066 - case hclrtLabSideAndQual
			case hclrtLabSideAndQual:
				aeiAuditItem = aeiHL7LabSideQualLink;
				break;
			//TES 6/3/2010 - PLID 38776 - Added hclrtLabResultField
			case hclrtLabResultField:
				aeiAuditItem = aeiHL7LabResultFieldLink;
				break;
			//TES 6/30/2010 - PLID 39320 - Added hclrtInsCo
			case hclrtInsCo:
				aeiAuditItem = aeiHL7InsCoLink;
				break;
			case hclrtResource: // (z.manning 2010-07-07 15:36) - PLID 39559
				aeiAuditItem = aeiHL7ResourceLink;
				break;
			//TES 7/13/2010 - PLID 39610 - Added hclrtRelationToPatient
			case hclrtRelationToPatient:
				aeiAuditItem = aeiHL7RelationToPatientLink;
				break;
			// (d.thompson 2012-08-21) - PLID 52048 - Added hclrtEthnicity
			case hclrtEthnicity:
				aeiAuditItem = aeiHL7EthnicityLink;
				break;
			// (d.thompson 2012-08-22) - PLID 52047 - Added hclrtRace
			case hclrtRace:
				aeiAuditItem = aeiHL7RaceLink;
				break;
			// (d.thompson 2012-08-23) - PLID 52049 - Added hclrtLanguage
			case hclrtLanguage:
				aeiAuditItem = aeiHL7LanguageLink;
				break;
				// (d.singleton 2012-08-28 14:31) - PLID 52302 - added hclrtInsuranceCategoryType
			case hclrtInsuranceCategoryType:
				aeiAuditItem = aeiHL7InsuranceCategoryTypeLink;
				break;
				// (r.farnworth 2015-01-20 13:42) - PLID 64624 - Add the ability to override Charge Codes when importing charges through HL7.
			case hclrtServiceCode:
				aeiAuditItem = aeiHL7ServiceLink;
				break;
				// (b.eyers 2015-06-04) - PLID 66205 - Added hclrtInventory
			case hclrtInventory:
				aeiAuditItem = aeiHL7InventoryLink;
				break;

			default:
				AfxThrowNxException("Invalid HL7 Record Type found in CHL7ConfigCodeLinksDlg");
				break;
		}
		CString strRecordName = GetHL7CodeLinkRecordName(m_hclrtType);

		//first delete what no longer exists
		//TES 9/16/2010 - PLID 40518 - We now have an array of just what was deleted.
		for(int i = 0; i < m_arDeletedCodeMaps.GetSize(); i++) {
			//not found, remove it
			CString strCode = m_arDeletedCodeMaps[i].strCode;
			//TES 9/16/2010 - PLID 40518 - We need to identify both the code and PracticeID being deleted, we can't count on the code
			// being a unique identifier.
			AddStatementToSqlBatch(strSql, "DELETE FROM HL7CodeLinkT "
				"WHERE HL7GroupID = %li AND Type = %i AND ThirdPartyCode = '%s' AND PracticeID = %li ", 
				m_nHL7GroupID, m_hclrtType, _Q(strCode), m_arDeletedCodeMaps[i].nPracticeID);
				
			// (j.jones 2010-05-12 10:24) - PLID 36527 - audit this
			if(nAuditTransactionID == -1) {
				nAuditTransactionID = BeginAuditTransaction();
			}
			CString strOld, strNew;
			//TES 9/16/2010 - PLID 40518 - Include the PracticeName in the Auditing, the code is not necessarily a unique identifier.
			//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
			strOld.Format("%s Code '%s', Linked to %s (HL7 Group '%s')", strRecordName, strCode, GetPracticeName(m_arDeletedCodeMaps[i].nPracticeID), GetHL7GroupName(m_nHL7GroupID));
			strNew.Format("<Code Removed>");
			AuditEvent(-1, "", nAuditTransactionID, aeiAuditItem, m_nHL7GroupID, strOld, strNew, aepLow, aetDeleted);
		}
		m_arDeletedCodeMaps.RemoveAll();

		//now add/update
		IRowSettingsPtr pRow = m_pCodeMapList->GetFirstRow();
		while(pRow) {
			CString strThirdPartyCode = VarString(pRow->GetValue(lcThirdPartyCode), "");
			long nPracticeID = VarLong(pRow->GetValue(lcPracticeID),-1);

			//is this setting new?
			//TES 9/16/2010 - PLID 40518 - We now store the old values in the datalist, rather than member variables.
			CString strOldCode = VarString(pRow->GetValue(lcOldThirdPartyCode), "");
			long nOldPracticeID = VarLong(pRow->GetValue(lcOldPracticeID),-1);

			if(!strOldCode.IsEmpty()) {
				//TES 9/16/2010 - PLID 40518 - Check whether it's changed.
				if(nOldPracticeID != nPracticeID || strOldCode != strThirdPartyCode) {
					CString strOldPracticeID = "Is Null";
					if(nOldPracticeID != -1 || m_hclrtType == hclrtLabSideAndQual) {
						strOldPracticeID.Format("= %li", nOldPracticeID);
					}

					CString strPracticeID;
					//TES 6/10/2010 - PLID 38066 - -1 is a valid PracticeID for hclrtLabSideAndQual.  Not the best decision I've ever made, 
					// but that ship has sailed.
					if (nPracticeID == -1 && m_hclrtType != hclrtLabSideAndQual) {
						strPracticeID = "NULL";
					}
					else {
						strPracticeID.Format("%li", nPracticeID);
					}

					AddStatementToSqlBatch(strSql, "UPDATE HL7CodeLinkT SET ThirdPartyCode = '%s', PracticeID = %s "
						"WHERE HL7GroupID = %li AND Type = %i AND ThirdPartyCode = '%s' AND PracticeID %s", _Q(strThirdPartyCode), 
						strPracticeID, m_nHL7GroupID, m_hclrtType, _Q(strOldCode), strOldPracticeID);

					// (j.jones 2010-05-12 10:24) - PLID 36527 - audit this
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					CString strOld, strNew;
					//TES 9/16/2010 - PLID 40518 - I added a long-overdue utility function to get the record name for a given ID.
					CString strOldPracticeName = GetPracticeName(nOldPracticeID);
					CString strNewPracticeName = GetPracticeName(nPracticeID);
					
					//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
					strOld.Format("%s Code '%s' (HL7 Group '%s')", strRecordName, strThirdPartyCode, GetHL7GroupName(m_nHL7GroupID));
					//TES 9/16/2010 - PLID 40518 - Include both the code and the linked record, both may have changed.
					CString strNewCode;
					if(strOldCode != strThirdPartyCode) {
						strNewCode.Format("Code changed from %s to %s", strOldCode, strThirdPartyCode);
					}
					CString strNewPracticeID;
					if(nOldPracticeID != nPracticeID) {
						strNewPracticeID.Format("Link changed from %s to %s", strOldPracticeName, strNewPracticeName);
					}
					ASSERT(!strNewCode.IsEmpty() || !strNewPracticeID.IsEmpty());
					strNew = strNewCode + ((!strNewCode.IsEmpty() && !strNewPracticeID.IsEmpty())?", ":"") + strNewPracticeID;
					AuditEvent(-1, "", nAuditTransactionID, aeiAuditItem, m_nHL7GroupID, strOld, strNew, aepLow, aetChanged);
				}
			}
			else {
				//create new entry
				// (j.jones 2015-11-16 10:25) - PLID 67491 - the insert statement is now created in a modular function
				AddStatementToSqlBatch(strSql, "%s ", CreateNewHL7CodeLinkT(m_nHL7GroupID, m_hclrtType, strThirdPartyCode, nPracticeID).Flatten());

				// (j.jones 2010-05-12 10:24) - PLID 36527 - audit this
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				//TES 9/16/2010 - PLID 40518 - I added a long-overdue utility function to get the record name for a given ID.
				CString strPracticeName = GetPracticeName(nPracticeID);

				CString strOld, strNew;
				//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
				strOld.Format("%s Code '%s' (HL7 Group '%s')", strRecordName, strThirdPartyCode, GetHL7GroupName(m_nHL7GroupID));
				strNew.Format("Linked to %s", strPracticeName);
				AuditEvent(-1, "", nAuditTransactionID, aeiAuditItem, m_nHL7GroupID, strOld, strNew, aepLow, aetCreated);
			}

			pRow = pRow->GetNextRow();
		}

		if(!strSql.IsEmpty()) {
			ExecuteSqlBatch(strSql);
		}

		if(nAuditTransactionID != -1) {
			CommitAuditTransaction(nAuditTransactionID);
			nAuditTransactionID = -1;
		}
		
		CNxDialog::OnOK();

	}NxCatchAllCall("Error in CHL7ConfigCodeLinksDlg::OnOK()",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	)
}

BEGIN_EVENTSINK_MAP(CHL7ConfigCodeLinksDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CHL7ConfigCodeLinksDlg)
	ON_EVENT(CHL7ConfigCodeLinksDlg, IDC_HL7_CODE_MAP, 2 /* SelChanged */, OnSelChangedHL7CodeMap, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CHL7ConfigCodeLinksDlg, IDC_HL7_CODE_MAP, 7 /* RButtonUp */, OnRButtonUpHL7CodeMap, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CHL7ConfigCodeLinksDlg, IDC_HL7_CODE_MAP, 9, CHL7ConfigCodeLinksDlg::OnEditingFinishingHL7CodeMap, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CHL7ConfigCodeLinksDlg::OnSelChangedHL7CodeMap(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	if(lpNewSel == NULL) {
		GetDlgItem(IDC_REMOVE_HL7_CODE_MAP)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_REMOVE_HL7_CODE_MAP)->EnableWindow(TRUE);
	}
}

void CHL7ConfigCodeLinksDlg::OnRButtonUpHL7CodeMap(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if(lpRow == NULL) return;
		IRowSettingsPtr pRow(lpRow);
		m_pCodeMapList->CurSel = pRow;
		GetDlgItem(IDC_REMOVE_HL7_CODE_MAP)->EnableWindow(TRUE);

		CMenu mnu;
		if (mnu.CreatePopupMenu()) {
			//Add our item.
			mnu.InsertMenu(-1, MF_BYPOSITION, 1, "Remove");
			CPoint pt;
			GetCursorPos(&pt);
			int nResult = mnu.TrackPopupMenu(TPM_NONOTIFY|TPM_RETURNCMD, pt.x, pt.y, this);
			//Did they select our item?
			if (nResult == 1) {
				OnRemoveHl7CodeMap();
			} else {
				//They didn't choose anything, so we don't need to do anything?
				ASSERT(nResult == 0);
				return;
			}
		} else {
			//Wha??
			ASSERT(FALSE);
		}
	}NxCatchAll("Error in CHL7ConfigCodeLinksDlg::OnRButtonUpCodeLinkMap()");
}

// (j.jones 2010-05-12 10:20) - PLID 36527 - this dialog never validated the data entered before
void CHL7ConfigCodeLinksDlg::OnEditingFinishingHL7CodeMap(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		if(*pbCommit == FALSE) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == lcThirdPartyCode) {

			CString strCode = strUserEntered;
			//TES 8/19/2010 - PLID 38899 - For result flags, we trim leading/trailing spaces when we import, thus our validation
			// needs to do the same.
			if(m_hclrtType == hclrtLabResultFlag) {
				strCode.TrimRight();
				strCode.TrimLeft();
				*pvarNewValue = _variant_t(strCode).Detach();
			}

			if(strCode.IsEmpty()) {
				AfxMessageBox("You cannot enter a blank code.");
				*pbContinue = FALSE;
				*pbCommit = FALSE;
				return;
			}
			else {

				//see if it already exists
				IRowSettingsPtr pFindRow = m_pCodeMapList->GetFirstRow();
				while(pFindRow) {

					//skip this row
					if(pFindRow == pRow) {
						pFindRow = pFindRow->GetNextRow();
						continue;
					}

					CString strFindThirdPartyCode = VarString(pFindRow->GetValue(lcThirdPartyCode), "");

					//case insensitive match
					if(strFindThirdPartyCode.CompareNoCase(strCode) == 0) {
						//TES 9/16/2010 - PLID 40518 - Insurance Companies allow multiple companies to be linked to the same code.
						if(m_hclrtType != hclrtInsCo) {
							AfxMessageBox("The code you entered already exists in the list.");
							*pbContinue = FALSE;
							*pbCommit = FALSE;
							return;
						}
						else {
							//TES 9/16/2010 - PLID 40518 - While we allow this, it could potentially cause problems when importing.  So,
							// if they're set to import insurance information, give them a warning, though we still allow them to continue.
							//TES 6/22/2011 - PLID 44261 - New functions for HL7 Settings
							if(GetHL7SettingBit(m_nHL7GroupID, "ImportInsurance")) {
								if(IDYES != MsgBox(MB_YESNO, "WARNING: The code you entered is already mapped to an existing Insurance Company in Practice.  "
									"If you map the same code to multiple Insurance Companies, any messages imported with that code may be linked with "
									"an unexpected Insurance Company.  Are you SURE you wish to continue?", "DuplicateHL7InsCoCode", "", FALSE, TRUE)) {
										*pbContinue = FALSE;
										*pbCommit = FALSE;
										return;
								}
								else {
									//TES 1/21/2011 - PLID 40518 - We've already warned them and they said it was cool, so there's
									// no need for us to keep looping.
									return;
								}
							}
						}
					}

					pFindRow = pFindRow->GetNextRow();
				}
			}
		}
	
	}NxCatchAll("Error in CHL7ConfigCodeLinksDlg::OnEditingFinishingCodeLinkMap()");
}

//TES 9/16/2010 - PLID 40518 - I added a long-overdue utility function to get the record name for a given ID.
CString CHL7ConfigCodeLinksDlg::GetPracticeName(long nPracticeID)
{
	_RecordsetPtr rsPracticeName;
	switch(m_hclrtType) {
		case hclrtLocation:
			if(nPracticeID == -1) {
				return "<Use Practice Default>";
			}
			rsPracticeName = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", nPracticeID);
			break;
		//TES 5/26/2010 - PLID 38660 - Added hclrtLabResultFlag
		case hclrtLabResultFlag:
			if(nPracticeID == -1) {
				return "<Use Practice Default>";
			}
			rsPracticeName = CreateParamRecordset("SELECT Name FROM LabResultFlagsT WHERE ID = {INT}", nPracticeID);
			break;
		//TES 6/1/2010 - PLID 38066 - Added hclrtLabSideAndQual
		case hclrtLabSideAndQual:
			if(nPracticeID == -1) {
				rsPracticeName = CreateParamRecordset("SELECT 'Left' AS Name");
			}
			else if(nPracticeID == -2) {
				rsPracticeName = CreateParamRecordset("SELECT 'Right' AS Name");
			}
			else {
				rsPracticeName = CreateParamRecordset("SELECT Name FROM AnatomyQualifiersT WHERE ID = {INT}", nPracticeID);
			}
			break;
		//TES 6/3/2010 - PLID 38776 - Added hclrtLabResultField
		case hclrtLabResultField:
			if(nPracticeID == -1) {
				return "<Use Practice Default>";
			}
			rsPracticeName = CreateParamRecordset("SELECT {STRING} AS Name", GetLabResultFieldName((HL7LabResultField)nPracticeID));
			break;
		//TES 6/30/2010 - PLID 39320 - Added hclrtInsCo
		case hclrtInsCo:
			if(nPracticeID == -1) {
				return "<Use Practice Default>";
			}
			rsPracticeName = CreateParamRecordset("SELECT Name FROM InsuranceCoT WHERE PersonID = {INT}", nPracticeID);
			break;
		case hclrtResource: // (z.manning 2010-07-07 15:37) - PLID 39559
			if(nPracticeID == -1) {
				return "<Use Practice Default>";
			}
			rsPracticeName = CreateParamRecordset("SELECT Item AS Name FROM ResourceT WHERE ID = {INT}", nPracticeID);
			break;
		//TES 7/13/2010 - PLID 39610 - Added hclrtRelationToPatient
		case hclrtRelationToPatient:
			if(nPracticeID == -1) {
				return "<Use Practice Default>";
			}
			rsPracticeName = CreateParamRecordset("SELECT {STRING} AS Name", GetRelation((RelationToPatient)nPracticeID));
			break;
		// (d.thompson 2012-08-21) - PLID 52048
		case hclrtEthnicity:
			rsPracticeName = CreateParamRecordset("SELECT Name FROM EthnicityT WHERE ID = {INT}", nPracticeID);
			break;
		// (d.thompson 2012-08-22) - PLID 52047
		case hclrtRace:
			rsPracticeName = CreateParamRecordset("SELECT Name FROM RaceT WHERE ID = {INT}", nPracticeID);
			break;
		// (d.thompson 2012-08-23) - PLID 52049
		case hclrtLanguage:
			rsPracticeName = CreateParamRecordset("SELECT Name FROM LanguageT WHERE ID = {INT}", nPracticeID);
			break;
		// (d.singleton 2012-08-28 14:44) - PLID 52302
		case hclrtInsuranceCategoryType:
			switch(nPracticeID)
			{
			case rctMedical:
				rsPracticeName = CreateParamRecordset("SELECT {STRING} AS Name", "Medical");
				break;
			case rctVision:
				rsPracticeName = CreateParamRecordset("SELECT {STRING} AS Name", "Vision");
				break;
			case rctAuto:
				rsPracticeName = CreateParamRecordset("SELECT {STRING} AS Name", "Auto");
				break;
			case rctWorkersComp:
				rsPracticeName = CreateParamRecordset("SELECT {STRING} AS Name", "Workers'' Comp.");
				break;
			case rctDental:
				rsPracticeName = CreateParamRecordset("SELECT {STRING} AS Name", "Dental");
				break;
			case rctStudy:
				rsPracticeName = CreateParamRecordset("SELECT {STRING} AS Name", "Study");
				break;
			case rctLOP:
				rsPracticeName = CreateParamRecordset("SELECT {STRING} AS Name", "Letter of Protection");
				break;
			case rctLOA:
				rsPracticeName = CreateParamRecordset("SELECT {STRING} AS Name", "Letter of Agreement");
				break;
			default:
				rsPracticeName = CreateParamRecordset("SELECT {STRING} AS Name", "Medical");
				break;
			}
			break;
		// (r.farnworth 2015-01-20 13:42) - PLID 64624 - Add the ability to override Charge Codes when importing charges through HL7.
		case hclrtServiceCode:
			rsPracticeName = CreateParamRecordset("SELECT Code AS Name FROM CPTCodeT WHERE ID = {INT}", nPracticeID);
			break;
		// (b.eyers 2015-06-08) - PLID 66205
		case hclrtInventory:
			rsPracticeName = CreateParamRecordset("SELECT Name FROM ServiceT WHERE ID = {INT}", nPracticeID);
			break;
		default:
			AfxThrowNxException("Invalid HL7 Record Type found in CHL7ConfigCodeLinksDlg");
			break;
	}

	CString strPracticeName;
	if(!rsPracticeName->eof) {
		strPracticeName = AdoFldString(rsPracticeName, "Name");
	}
	else {
		strPracticeName = "<Use Practice Default>";
	}
	return strPracticeName;
}

// (r.gonet 09/27/2011) - PLID 45719 - Import some HL7 link codes from a CSV file.
void CHL7ConfigCodeLinksDlg::OnBnClickedImportHl7CodeMap()
{
	try { 
		std::multimap<long, CString> mapUnmatchedCodes;
		// Open the importer
		CHL7CodeLinkImportDlg dlg(this);
		if(IDOK == dlg.DoModal()) {
			// Ok, we should have some parsed codes and some fields to draw from.
			boost::shared_ptr<CCSVRecordSet> pCSVRecordSet = dlg.GetCSVRecordSet();
			// Go through the parsed records and try to match them up with existing codes. If they match, great, update the existing code.
			//  If they don't match anything though, add a new entry.
			for(int i = 0; i < pCSVRecordSet->GetRecordCount(); i++) {
				// Get the code and ID pair for this record
				CString strThirdPartyCode = pCSVRecordSet->GetFieldValue(i, dlg.GetThirdPartyCodeColumn());
				CString strPracticeID = pCSVRecordSet->GetFieldValue(i, dlg.GetPracticeIDColumn());
				long nPracticeID = atol(strPracticeID);

				// Try to find a existing match.
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCodeMapList->FindByColumn(lcPracticeID, _variant_t(nPracticeID, VT_I4), m_pCodeMapList->GetFirstRow(), VARIANT_FALSE);
				if(pRow) {
					// It matches something, so update the existing value.
					pRow->PutValue(lcThirdPartyCode, _variant_t(strThirdPartyCode));
				} else {
					// Nope.  Let's add it.
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCodeMapList->GetNewRow();
					pRow->PutValue(lcThirdPartyCode, _variant_t(strThirdPartyCode));
					pRow->PutValue(lcPracticeID, _variant_t((long)nPracticeID, VT_I4));
					pRow->PutValue(lcOldThirdPartyCode, g_cvarNull);
					pRow->PutValue(lcOldPracticeID, g_cvarNull);
					_variant_t varOutputValue = pRow->GetOutputValue(lcPracticeID);
					// See if this value is valid (ie contained in the embedded combo)
					if(varOutputValue.vt != VT_NULL && varOutputValue.vt != VT_EMPTY) {
						m_pCodeMapList->AddRowAtEnd(pRow, NULL);
						GetDlgItem(IDC_REMOVE_HL7_CODE_MAP)->EnableWindow(TRUE);
					} else {
						// Then what is it? Inactive or invalid? We'll check in a bit.
						mapUnmatchedCodes.insert(std::pair<long, CString>(nPracticeID, strThirdPartyCode));
					}
				}
			}
			
			if(mapUnmatchedCodes.size() > 0) {
				// We have some left over imported links that we are not sure what to do with.
				//  There is no corresponding value in the embedded Practice ID combo. We must decide if
				//  these are inactive or are invalid Practice IDs. There is only one way to do that. Query.
				CArray<long, long> aryUnmatchedPracticeIDs;
				std::multimap<long, CString>::iterator iter;
				for(iter = mapUnmatchedCodes.begin(); iter != mapUnmatchedCodes.end(); iter++) {
					aryUnmatchedPracticeIDs.Add(iter->first);
				}
				
				// (r.gonet 09/27/2011) - PLID 45719 - Currently we only support insurance companies anyway.
				CParamSqlBatch sqlIDQuery;
				if(m_hclrtType == hclrtInsCo) {
					sqlIDQuery.Add(
						"SELECT PersonID AS PracticeID FROM InsuranceCoT WHERE PersonID IN ({INTARRAY}); ",
						aryUnmatchedPracticeIDs);
				} else {
					ASSERT(FALSE);
					return;
				}
				if(!sqlIDQuery.IsEmpty()) {
					_RecordsetPtr prs = sqlIDQuery.CreateRecordset(GetRemoteData());
					while(!prs->eof) {
						long nFoundPracticeID = VarLong(prs->Fields->Item["PracticeID"]->Value);
						
						// Find all third party codes with this practice ID
						std::pair<std::multimap<long, CString>::iterator, std::multimap<long, CString>::iterator> range = 
							mapUnmatchedCodes.equal_range(nFoundPracticeID);
						std::multimap<long, CString>::iterator codeIter;
						for(codeIter = range.first; codeIter != range.second; codeIter++) {
							CString strFoundThirdPartyCode = codeIter->second;
							// Now we know that this link is a valid combination. It was just inactive and not included in the embedded combo.
							//  The only thing we can't do is show the text ... I think.
							NXDATALIST2Lib::IRowSettingsPtr pRow = m_pCodeMapList->GetNewRow();
							pRow->PutValue(lcThirdPartyCode, _variant_t(strFoundThirdPartyCode));
							pRow->PutValue(lcPracticeID, _variant_t((long)nFoundPracticeID, VT_I4));
							pRow->PutValue(lcOldThirdPartyCode, g_cvarNull);
							pRow->PutValue(lcOldPracticeID, g_cvarNull);
							pRow->Visible = VARIANT_TRUE; // This row would show up with no combo box output text, so it is better to set it to non-visible.
							m_pCodeMapList->AddRowAtEnd(pRow, NULL);
							GetDlgItem(IDC_REMOVE_HL7_CODE_MAP)->EnableWindow(TRUE);
						}

						prs->MoveNext();
					}
					prs->Close();
				}
			}
			MessageBox("Import Finished!", 0, MB_ICONINFORMATION);
		}
	} NxCatchAll(__FUNCTION__);
}