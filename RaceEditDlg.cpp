// RaceEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RaceEditDlg.h"
#include "AuditTrail.h"

// CRaceEditDlg dialog

// (j.jones 2009-10-15 09:02) - PLID 34327 - created

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37026 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



enum RaceListColumns {

	rlcID = 0,
	rlcName,
	rlcRaceCode,
};

CRaceEditDlg::CRaceEditDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CRaceEditDlg::IDD, pParent)
{

}

void CRaceEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_ADD_RACE, m_btnAdd);
	DDX_Control(pDX, IDC_BTN_DELETE_RACE, m_btnDelete);
	DDX_Control(pDX, IDC_BTN_CLOSE_RACE, m_btnClose);
}


BEGIN_MESSAGE_MAP(CRaceEditDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_ADD_RACE, OnBtnAddRace)
	ON_BN_CLICKED(IDC_BTN_DELETE_RACE, OnBtnDeleteRace)
	ON_BN_CLICKED(IDC_BTN_CLOSE_RACE, OnBtnCloseRace)
END_MESSAGE_MAP()


// CRaceEditDlg message handlers
BOOL CRaceEditDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);

		m_RaceList = BindNxDataList2Ctrl(IDC_RACE_LIST);

	}NxCatchAll("Error in CRaceEditDlg::OnInitDialog()");

	return FALSE;
}

void CRaceEditDlg::OnBtnAddRace()
{
	try {

		CString strResult;
		if(InputBox(this, "Please enter a new race name", strResult, "") == IDOK) {
			
			if(strResult.IsEmpty()) {
				return;
			}

			//enforce the length limit
			if(strResult.GetLength() > 255) {
				AfxMessageBox("You entered a race name greater than the maximum length (255). The name will be truncated.");
				strResult = strResult.Left(255);
			}

			//enforce a unique name
			_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM RaceT WHERE Name = {STRING}", strResult);
			if(!rs->eof) {
				AfxMessageBox("There is already a race entry with this name.");
				return;
			}

			//save this name
			long nID = NewNumber("RaceT", "ID");
			ExecuteParamSql("INSERT INTO RaceT (ID, Name) VALUES ({INT}, {STRING})", nID, strResult);
			
			IRowSettingsPtr pRow = m_RaceList->GetNewRow();
			pRow->PutValue(rlcID, nID);
			pRow->PutValue(rlcName,_bstr_t(strResult));
			pRow->PutValue(rlcRaceCode, g_cvarNull);
			m_RaceList->AddRowSorted(pRow, NULL);
			m_RaceList->PutCurSel(pRow);

			//audit this
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, "", nAuditID, aeiRaceCreated, nID, "", strResult, aepLow, aetCreated);
		}

	}NxCatchAll("Error in CRaceEditDlg::OnBtnAddRace()");
}

void CRaceEditDlg::OnBtnDeleteRace()
{
	try {

		IRowSettingsPtr pRow = m_RaceList->GetCurSel();
		if(pRow == NULL) {
			AfxMessageBox("No race entry is selected.");
			return;
		}

		long nID = VarLong(pRow->GetValue(rlcID));
		CString strName = VarString(pRow->GetValue(rlcName));

		// (b.spivey, May 20, 2013) - PLID 56925 - Check to see if we have any patients at all with this race. 
		_RecordsetPtr rs = CreateParamRecordset("SELECT Count(PersonID) AS CountPatients FROM PersonRaceT WHERE RaceID = {INT}", nID);
		if(!rs->eof) {
			long nCount = AdoFldLong(rs, "CountPatients", 0);
			if(nCount > 0) {
				CString str;
				str.Format("You may not delete this race description because there are %li patient(s) associated with it.", nCount);
				AfxMessageBox(str);
				return;
			}
		}
		rs->Close();

		//(e.lally 2006-01-18) PLID 18885 - We have a table now that references the RaceT.ID, we can delete that reference in
		//Tops_EthnicityLinkT without prompting. (j.jones 2009-10-19 10:27) - this table is outdated, but still exists)
		ExecuteParamSql("DELETE FROM Tops_EthnicityLinkT WHERE PracEthnicityID = {INT}", nID);
		//(e.lally 2011-05-05) PLID 43481 - remove NexWeb display setup
		ExecuteParamSql("DELETE FROM NexWebDisplayT WHERE RaceID = {INT}", nID);
		ExecuteParamSql("DELETE FROM RaceT WHERE ID = {INT}", nID);

		//audit this
		long nAuditID = BeginNewAuditEvent();
		AuditEvent(-1, "", nAuditID, aeiRaceDeleted, nID, strName, "<Deleted>", aepLow, aetDeleted);

		m_RaceList->RemoveRow(pRow);

	}NxCatchAll("Error in CRaceEditDlg::OnBtnDeleteRace()");
}

void CRaceEditDlg::OnBtnCloseRace()
{
	try {

		CNxDialog::OnOK();

	}NxCatchAll("Error in CRaceEditDlg::OnBtnCloseRace()");
}

BEGIN_EVENTSINK_MAP(CRaceEditDlg, CNxDialog)
	ON_EVENT(CRaceEditDlg, IDC_RACE_LIST, 9, OnEditingFinishingRaceList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CRaceEditDlg, IDC_RACE_LIST, 10, OnEditingFinishedRaceList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
END_EVENTSINK_MAP()

void CRaceEditDlg::OnEditingFinishingRaceList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		long nID = VarLong(pRow->GetValue(rlcID));

		if(*pbCommit && nCol == rlcName) {
			CString strValue = VarString(pvarNewValue, "");
			strValue.TrimLeft();
			strValue.TrimRight();
			if(strValue.IsEmpty()) {
				AfxMessageBox("You cannot enter a blank name.");
				*pbCommit = FALSE;
				return;
			}

			if(strValue.GetLength() > 255) {
				//should be impossible as the datalist should be enforcing this
				AfxMessageBox("You entered a race name greater than the maximum length (255). The name will be truncated.");
				strValue = strValue.Left(255);
				*pvarNewValue = _variant_t(strValue).Detach();
			}

			if(VarString(varOldValue, "") != strValue) {
				_RecordsetPtr rs = CreateParamRecordset("SELECT ID FROM RaceT WHERE Name = {STRING} AND ID <> {INT}", strUserEntered, nID);
				if(!rs->eof) {
					AfxMessageBox("There is already a race entry with this name.");
					*pbCommit = FALSE;
					return;
				}
			}
		}

		if(*pbCommit && (nCol == rlcName || nCol == rlcRaceCode)) {
			if(pvarNewValue->vt == VT_I4 && varOldValue.vt == VT_I4 && VarLong(pvarNewValue) == VarLong(varOldValue)) {
				*pbCommit = FALSE;
				return;
			}
			else if(pvarNewValue->vt == VT_BSTR && varOldValue.vt == VT_BSTR && VarString(pvarNewValue) == VarString(varOldValue)) {
				*pbCommit = FALSE;
				return;
			}

			// (b.spivey, May 28, 2013) - PLID 56925 - Changed this to use the new table and column. 
			_RecordsetPtr rs = CreateParamRecordset("SELECT Count(PersonID) AS CountPatients FROM PersonRaceT WHERE RaceID = {INT}", nID);
			if(!rs->eof) {
				long nCount = AdoFldLong(rs, "CountPatients", 0);
				if(nCount > 0) {
					CString str;
					str.Format("This race entry is in use by %li patient(s). Are you sure you wish to change it?", nCount);
					if(IDNO == MessageBox(str, "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {
						*pbCommit = FALSE;
						return;
					}				
				}
			}
			rs->Close();
		}

	}NxCatchAll("Error in CRaceEditDlg::OnEditingFinishingRaceList()");
}

void CRaceEditDlg::OnEditingFinishedRaceList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(!bCommit) {
			return;
		}

		long nID = VarLong(pRow->GetValue(rlcID));

		//for auditing purposes
		CString strOldName = "";
		long nOldRaceCodeID = -1;
		CString strOldRace = "<None>";
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, RaceCodeID, OfficialRaceName "
			"FROM RaceT "
			"LEFT JOIN RaceCodesT ON RaceT.RaceCodeID = RaceCodesT.ID "
			"WHERE RaceT.ID = {INT}", nID);
		if(!rs->eof) {
			strOldName = AdoFldString(rs, "Name", "");
			nOldRaceCodeID = AdoFldLong(rs, "RaceCodeID", -1);
			strOldRace = AdoFldString(rs, "OfficialRaceName", "<None>");
		}
		rs->Close();

		if(nCol == rlcName) {
			CString strValue = VarString(varNewValue, "");
			strValue.TrimLeft();
			strValue.TrimRight();
			pRow->PutValue(rlcName, _bstr_t(strValue));
			if(strValue != strOldName) {
				ExecuteParamSql("UPDATE RaceT SET Name = {STRING} WHERE ID = {INT}", strValue, nID);

				//audit this
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiRaceRenamed, nID, strOldName, strValue, aepMedium, aetChanged);
			}
		}
		else if(nCol == rlcRaceCode) {
			_variant_t varToSave = g_cvarNull;			
			long nCodeToSave = -1;
			if(varNewValue.vt == VT_I4 && VarLong(varNewValue) != -1) {
				varToSave = varNewValue;
				nCodeToSave = VarLong(varToSave);
			}
			if(nCodeToSave != nOldRaceCodeID) {
				ExecuteParamSql("UPDATE RaceT SET RaceCodeID = {VT_I4} WHERE ID = {INT}", varToSave, nID);

				//get the new race name for auditing
				CString strNew = "<None>";
				if(varToSave.vt == VT_I4 && VarLong(varToSave) != -1) {
					_RecordsetPtr rsRace = CreateParamRecordset("SELECT OfficialRaceName FROM RaceCodesT WHERE ID = {INT}", VarLong(varToSave));
					if(!rsRace->eof) {
						strNew = AdoFldString(rsRace, "OfficialRaceName", "<None>");
					}
					rsRace->Close();
				}

				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiRaceRaceCode, nID, strOldRace, strNew, aepMedium, aetChanged);
			}
			
			if(varToSave.vt == VT_NULL) {
				//clear the selection
				pRow->PutValue(rlcRaceCode, varToSave);
			}
		}

	}NxCatchAll("Error in CRaceEditDlg::OnEditingFinishedRaceList()");
}