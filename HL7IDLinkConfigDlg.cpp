// HL7IDLinkConfigDlg.cpp : implementation file
//

// (j.dinatale 2013-01-08 10:48) - PLID 54491 - created

#include "stdafx.h"
#include "Practice.h"
#include "FinancialRc.h"
#include "HL7IDLinkConfigDlg.h"
#include "HL7Utils.h"
#include <NxHL7Lib\HL7CommonTypes.h>
#include <NxHL7Lib\HL7CommonUtils.h>
#include "AuditTrail.h"
#include "HL7CodeLinkImportDlg.h"
#include <set>

using namespace ADODB;
using namespace NXDATALIST2Lib;

namespace HL7IDConfigList {
	enum HL7IDConfigCol {
		ThirdPartyID = 0,
		PersonID,
		OldThirdPartyID,
		OldPersonID,
	};
};

// CHL7IDLinkConfigDlg dialog

IMPLEMENT_DYNAMIC(CHL7IDLinkConfigDlg, CNxDialog)

CHL7IDLinkConfigDlg::CHL7IDLinkConfigDlg(HL7IDLink_RecordType hl7IDLinkType, long nHL7GroupID, CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7IDLinkConfigDlg::IDD, pParent)
{
	m_IDLinkRecordType = hl7IDLinkType;
	m_nHL7GroupID = nHL7GroupID;
}

CHL7IDLinkConfigDlg::~CHL7IDLinkConfigDlg()
{
}

void CHL7IDLinkConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_IMPORT_HL7_ID_MAP, m_btnImport);
	DDX_Control(pDX, IDC_ADD_HL7_ID_MAP, m_btnAdd);
	DDX_Control(pDX, IDC_REMOVE_HL7_ID_MAP, m_btnRemove);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}

BEGIN_MESSAGE_MAP(CHL7IDLinkConfigDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD_HL7_ID_MAP, &CHL7IDLinkConfigDlg::OnBnClickedAddHl7IdMap)
	ON_BN_CLICKED(IDC_REMOVE_HL7_ID_MAP, &CHL7IDLinkConfigDlg::OnBnClickedRemoveHl7IdMap)
	ON_BN_CLICKED(IDOK, &CHL7IDLinkConfigDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_IMPORT_HL7_ID_MAP, &CHL7IDLinkConfigDlg::OnBnClickedImportHl7IdMap)
END_MESSAGE_MAP()

// CHL7IDLinkConfigDlg message handlers
BOOL CHL7IDLinkConfigDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_btnImport.AutoSet(NXB_IMPORTBOX);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		SetWindowText("Configure HL7 Mapping For " + GetHL7IDLinkRecordName(m_IDLinkRecordType) + " IDs");
		m_pIDMapList = BindNxDataList2Ctrl(IDC_HL7_ID_MAP, false);

		CString strTable = GetTableName(m_IDLinkRecordType);

		// we include inactive objects here on purpose.
		if(!strTable.IsEmpty()){
			CSqlFragment sql(
				"SELECT {CONST_STRING}.PersonID, "
				"CASE WHEN PersonT.Archived = 0 THEN PersonT.FullName ELSE PersonT.FullName + ' (Inactive)' END AS Name "
				"FROM {CONST_STRING} "
				"INNER JOIN PersonT ON {CONST_STRING}.PersonID = PersonT.ID "
				"ORDER BY PersonT.Archived, Name ", strTable, strTable, strTable, m_IDLinkRecordType, strTable
			);

			IColumnSettingsPtr pCol = m_pIDMapList->GetColumn(HL7IDConfigList::PersonID);
			pCol->PutComboSource(_bstr_t(sql.Flatten()));
		}

		m_pIDMapList->WhereClause = _bstr_t(FormatString("RecordType = %li AND GroupID = %li", m_IDLinkRecordType, m_nHL7GroupID));
		m_pIDMapList->Requery();

		GetDlgItem(IDC_ADD_HL7_ID_MAP)->EnableWindow(TRUE);
		GetDlgItem(IDC_REMOVE_HL7_ID_MAP)->EnableWindow(m_pIDMapList->CurSel == NULL ? FALSE : TRUE);

	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CString CHL7IDLinkConfigDlg::GetTableName(HL7IDLink_RecordType hilrt)
{
	switch(m_IDLinkRecordType) {
		case hilrtPatient:
			return "PatientsT";
		case hilrtProvider:
			return "ProvidersT";
			break;
		case hilrtReferringPhysician:
			return "ReferringPhysT";
			break;
		default:
			ASSERT(FALSE); // we added a type and didnt account for it
			ThrowNxException("Unsupported HL7 ID Link Record Type.");
	}

	return "";
}

CString CHL7IDLinkConfigDlg::GetHL7IDLinkRecordName(HL7IDLink_RecordType hilrt)
{
	switch(hilrt) {
		case hilrtPatient:
			return "Patient";
		case hilrtProvider:
			return "Provider";
		case hilrtReferringPhysician:
			return "Referring Physician";
		default:
			ASSERT(FALSE);	// added a new type, need to support it
			ThrowNxException("Cannot get HL7 object name. Unsupported HL7 ID Link Record Type.");
	}

	return "";
}

void CHL7IDLinkConfigDlg::OnBnClickedAddHl7IdMap()
{
	try{
		bool bDone = false;
		CString strRecordName = GetHL7IDLinkRecordName(m_IDLinkRecordType);
		CString strInputDesc;
		strInputDesc.Format("Enter the %s ID you wish to link: ", strRecordName);

		// request user input
		while(!bDone){
			CString strCode;
			if(InputBoxLimited(this, strInputDesc, strCode, "", 255, false, false, NULL) == IDOK){
				if(!strCode.IsEmpty()){
					// check for duplicates
					IRowSettingsPtr pRow = m_pIDMapList->GetFirstRow();
					bool bFound = false;
					while(pRow){
						CString strRowID = VarString(pRow->GetValue(HL7IDConfigList::ThirdPartyID), "");
						if(!strCode.CompareNoCase(strRowID)){
							// we have a duplicate
							MsgBox("The ID you entered is already mapped to an existing " + strRecordName + " in Practice.  Please enter a new ID, "
									"or if you wish to change the mapping for this ID, click in the 'Practice Code' column to select "
									"a different " + strRecordName + ".");
							bFound = true;
							break;
						}

						pRow = pRow->GetNextRow();
					}

					if(!bFound){
						IRowSettingsPtr pRow = m_pIDMapList->GetNewRow();
						pRow->PutValue(HL7IDConfigList::ThirdPartyID, _bstr_t(strCode));
						pRow->PutValue(HL7IDConfigList::PersonID, g_cvarNull);
						pRow->PutValue(HL7IDConfigList::OldThirdPartyID, g_cvarNull);
						pRow->PutValue(HL7IDConfigList::OldPersonID, g_cvarNull);
						m_pIDMapList->AddRowAtEnd(pRow, NULL);
						m_pIDMapList->CurSel = pRow;
						GetDlgItem(IDC_REMOVE_HL7_ID_MAP)->EnableWindow(TRUE);
						m_pIDMapList->StartEditing(pRow, HL7IDConfigList::PersonID);
						bDone = true;	// we added our row, so bail.
					}
				}
			}else{
				bDone = true; // they canceled
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CHL7IDLinkConfigDlg::OnBnClickedRemoveHl7IdMap()
{
	try{
		IRowSettingsPtr pRow = m_pIDMapList->CurSel;
		if(pRow){
			CString strOldCode = VarString(pRow->GetValue(HL7IDConfigList::OldThirdPartyID), "");
			long nOldPersonID = VarLong(pRow->GetValue(HL7IDConfigList::OldPersonID), -1);

			if(!strOldCode.IsEmpty()) {
				HL7IDMap idMap;
				idMap.strCode = strOldCode;
				idMap.nPersonID = nOldPersonID;
				m_aryDeletedIDs.Add(idMap);
			}

			m_pIDMapList->RemoveRow(pRow);
		}
	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CHL7IDLinkConfigDlg, CNxDialog)
	ON_EVENT(CHL7IDLinkConfigDlg, IDC_HL7_ID_MAP, 2, CHL7IDLinkConfigDlg::SelChangedHl7IdMap, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CHL7IDLinkConfigDlg, IDC_HL7_ID_MAP, 9, CHL7IDLinkConfigDlg::EditingFinishingHl7IdMap, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CHL7IDLinkConfigDlg::SelChangedHl7IdMap(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	GetDlgItem(IDC_REMOVE_HL7_ID_MAP)->EnableWindow(m_pIDMapList->CurSel ? TRUE : FALSE);
}

void CHL7IDLinkConfigDlg::EditingFinishingHl7IdMap(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try{
		if(nCol != HL7IDConfigList::ThirdPartyID){
			return;
		}

		if(*pbCommit == FALSE) {
			return;
		}

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		CString strCode = strUserEntered;
		strCode.Trim();
		if(strCode.IsEmpty()) {
			AfxMessageBox("You cannot enter a blank code.");
			*pbContinue = FALSE;
			*pbCommit = FALSE;
			return;
		}

		IRowSettingsPtr pFindRow = m_pIDMapList->GetFirstRow();
		while(pFindRow) {
			//skip this row
			if(pFindRow == pRow) {
				pFindRow = pFindRow->GetNextRow();
				continue;
			}

			CString strFindThirdPartyCode = VarString(pFindRow->GetValue(HL7IDConfigList::ThirdPartyID), "");

			//case insensitive match
			if(!strFindThirdPartyCode.CompareNoCase(strCode)) {
				// we have a duplicate
				AfxMessageBox("The ID you entered already exists in the list.");
				*pbContinue = FALSE;
				*pbCommit = FALSE;
				return;
			}

			pFindRow = pFindRow->GetNextRow();
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2013-01-08 11:22) - PLID 54492 - saving and auditing
CString CHL7IDLinkConfigDlg::GetPersonName(long nPersonID)
{
	CString strName;
	_RecordsetPtr rs = CreateParamRecordset("SELECT PersonT.FullName FROM PersonT WHERE ID = {INT}", nPersonID);
	if(!rs->eof){
		strName = AdoFldString(rs, "FullName", "");
	}

	return strName;
}

// (j.dinatale 2013-01-14 16:49) - PLID 54492 - when the user hits ok, then we save and audit
void CHL7IDLinkConfigDlg::OnBnClickedOk()
{
	try{
		CSqlFragment sql;
		CAuditTransaction auditTrans;
		CString strRecordName = GetHL7IDLinkRecordName(m_IDLinkRecordType);
		AuditEventItems aeiAuditItem = (AuditEventItems)-1;

		switch(m_IDLinkRecordType) {
			case hilrtPatient:
				aeiAuditItem = aeiHL7PatientLink;
				break;
			case hilrtProvider:
				aeiAuditItem = aeiHL7ProviderLink;
				break;
			case hilrtReferringPhysician:
				aeiAuditItem = aeiHL7ReferringPhysicianLink;
				break;
			default:
				ASSERT(FALSE); // we added a type and didnt account for it
				ThrowNxException("Unsupported HL7 ID Link Record Type.");
		}

		for(int i = 0; i < m_aryDeletedIDs.GetSize(); i++) {
			CString strCode = m_aryDeletedIDs[i].strCode;

			sql += CSqlFragment(
				"DELETE FROM HL7IDLinkT WHERE GroupID = {INT} AND RecordType = {INT} AND ThirdPartyID = {STRING} AND PersonID = {INT} ", 
				m_nHL7GroupID, m_IDLinkRecordType, strCode, m_aryDeletedIDs[i].nPersonID
			);

			CString strOld, strNew;
			strOld.Format("%s ID '%s', Linked to %s (HL7 Group '%s')", strRecordName, strCode, GetPersonName(m_aryDeletedIDs[i].nPersonID), GetHL7GroupName(m_nHL7GroupID));
			strNew.Format("<Code Removed>");
			AuditEvent(-1, "", auditTrans, aeiAuditItem, m_nHL7GroupID, strOld, strNew, aepLow, aetDeleted);
		}

		IRowSettingsPtr pRow = m_pIDMapList->GetFirstRow();
		while(pRow){
			CString strThirdPartyCode = VarString(pRow->GetValue(HL7IDConfigList::ThirdPartyID), "");
			long nPersonID = VarLong(pRow->GetValue(HL7IDConfigList::PersonID), -1);
			CString strOldCode = VarString(pRow->GetValue(HL7IDConfigList::OldThirdPartyID), "");
			long nOldPersonID = VarLong(pRow->GetValue(HL7IDConfigList::OldPersonID), -1);

			// bad bad user...
			if(nPersonID < 0){
				CString strMessage;
				strMessage.Format("Please ensure that all codes have a %s assigned to them.", GetHL7IDLinkRecordName(m_IDLinkRecordType).MakeLower());
				this->MessageBox(strMessage, "Error!", MB_OK | MB_ICONERROR);
				return;
			}

			if(strOldCode.IsEmpty()){
				// new code
				sql += GetDeleteHL7IDLinkSql(m_nHL7GroupID, m_IDLinkRecordType, strThirdPartyCode);
				sql += CSqlFragment(
					"INSERT INTO HL7IDLinkT (ID, GroupID, ThirdPartyID, PersonID, RecordType) "
					"SELECT COALESCE(Max(ID),0) + 1, {INT}, {STRING}, {VT_I4}, {INT} FROM HL7IDLinkT ",
					m_nHL7GroupID, strThirdPartyCode, (nPersonID > 0 ? _variant_t(nPersonID) : g_cvarNull), m_IDLinkRecordType
				);

				CString strOld, strNew;
				strOld.Format("%s Code '%s' (HL7 Group '%s')", strRecordName, strThirdPartyCode, GetHL7GroupName(m_nHL7GroupID));
				strNew.Format("Linked to %s", GetPersonName(nPersonID));
				AuditEvent(-1, "", auditTrans, aeiAuditItem, m_nHL7GroupID, strOld, strNew, aepLow, aetCreated);
			}else{
				// old code
				if(strOldCode.CompareNoCase(strThirdPartyCode) != 0 || nOldPersonID != nPersonID){
					// that did change....
					CSqlFragment sqlPersonID;
					if(nOldPersonID > 0){
						sqlPersonID.Create("PersonID = {INT}", nOldPersonID);
					}else{
						sqlPersonID += "PersonID IS NULL";
					}

					sql += GetDeleteHL7IDLinkSql(m_nHL7GroupID, m_IDLinkRecordType, strThirdPartyCode);
					sql += CSqlFragment(
						"UPDATE HL7IDLinkT SET ThirdPartyID = {STRING}, PersonID = {INT} "
						"WHERE GroupID = {INT} AND RecordType = {INT} AND {SQL} AND ThirdPartyID = {STRING} ",
						strThirdPartyCode, nPersonID, m_nHL7GroupID, m_IDLinkRecordType, sqlPersonID, strOldCode
					);


					CString strNew, strOld;
					strOld.Format("%s Code '%s' (HL7 Group '%s')", strRecordName, strThirdPartyCode, GetHL7GroupName(m_nHL7GroupID));
					CString strNewCode, strNewPracticeID;

					if(strOldCode.CompareNoCase(strThirdPartyCode) != 0) {
						strNewCode.Format("Code changed from %s to %s", strOldCode, strThirdPartyCode);
					}

					if(nOldPersonID != nPersonID) {
						strNewPracticeID.Format("Link changed from %s to %s", GetPersonName(nOldPersonID), GetPersonName(nPersonID));
					}

					ASSERT(!strNewCode.IsEmpty() || !strNewPracticeID.IsEmpty());

					strNew = strNewCode + ((!strNewCode.IsEmpty() && !strNewPracticeID.IsEmpty()) ? ", " : "") + strNewPracticeID;
					AuditEvent(-1, "", auditTrans, aeiAuditItem, m_nHL7GroupID, strOld, strNew, aepLow, aetChanged);
				}
			}

			pRow = pRow->GetNextRow();
		}

		if(!sql.IsEmpty()){
			ExecuteParamSql("BEGIN TRAN {SQL} COMMIT TRAN", sql);
			auditTrans.Commit();
		}

		// now we can remove all of these
		m_aryDeletedIDs.RemoveAll();

		CNxDialog::OnOK();
	}NxCatchAll(__FUNCTION__);
}

// (j.dinatale 2013-01-14 16:49) - PLID 54602 - need to allow users to import ID mappings to make things easier
void CHL7IDLinkConfigDlg::OnBnClickedImportHl7IdMap()
{
	try{
		CHL7CodeLinkImportDlg dlg(this);
		if(IDOK == dlg.DoModal()) {
			boost::shared_ptr<CCSVRecordSet> pCSVRecordSet = dlg.GetCSVRecordSet();
			
			for(int i = 0; i < pCSVRecordSet->GetRecordCount(); i++) {
				// Get the code and ID pair for this record
				CString strThirdPartyCode = pCSVRecordSet->GetFieldValue(i, dlg.GetThirdPartyCodeColumn());
				strThirdPartyCode.Trim();
				
				// skip if we have a blank code
				if(strThirdPartyCode.IsEmpty()){
					continue;
				}

				CString strPersonID = pCSVRecordSet->GetFieldValue(i, dlg.GetPracticeIDColumn());
				long nPersonID = atol(strPersonID);

				// and we dont have a valid person ID, why bother?
				if(nPersonID <= 0){
					continue;
				}

				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pIDMapList->FindByColumn(HL7IDConfigList::ThirdPartyID, _bstr_t(strThirdPartyCode), NULL, FALSE);
				if(pRow){
					_variant_t vtOldPersonID = pRow->GetValue(HL7IDConfigList::PersonID);
					// found our selection, update it with the person object we are interested in
					pRow->PutValue(HL7IDConfigList::PersonID, nPersonID);

					_variant_t varOutputValue = pRow->GetOutputValue(HL7IDConfigList::PersonID);
					if(varOutputValue.vt == VT_NULL || varOutputValue.vt == VT_EMPTY){
						// opps we have a ref phys ID that doesnt exist, put the old PersonID back
						pRow->PutValue(HL7IDConfigList::PersonID, vtOldPersonID);
					}
				}else{
					// we found nothing! so we need to add a new row
					NXDATALIST2Lib::IRowSettingsPtr pRow = m_pIDMapList->GetNewRow();
					pRow->PutValue(HL7IDConfigList::ThirdPartyID, _variant_t(strThirdPartyCode));
					pRow->PutValue(HL7IDConfigList::PersonID, nPersonID);
					pRow->PutValue(HL7IDConfigList::OldPersonID, g_cvarNull);
					pRow->PutValue(HL7IDConfigList::OldThirdPartyID, g_cvarNull);

					_variant_t varOutputValue = pRow->GetOutputValue(HL7IDConfigList::PersonID);
					if(varOutputValue.vt == VT_NULL || varOutputValue.vt == VT_EMPTY){
						// opps we have a ref phys ID that doesnt exist, should we still add a row in this instance?
						pRow->PutValue(HL7IDConfigList::PersonID, g_cvarNull);
					}

					m_pIDMapList->AddRowAtEnd(pRow, NULL);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}
