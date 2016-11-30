// HL7ConfigLocationsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "HL7ConfigLocationsDlg.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CHL7ConfigLocationsDlg dialog

using namespace ADODB;
using namespace NXDATALIST2Lib;

enum ListColumns {

	lcThirdPartyID = 0,
	lcLocationID,
};

CHL7ConfigLocationsDlg::CHL7ConfigLocationsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CHL7ConfigLocationsDlg::IDD, pParent)
{
	m_nHL7GroupID = -1;
	//{{AFX_DATA_INIT(CHL7ConfigLocationsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CHL7ConfigLocationsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CHL7ConfigLocationsDlg)
	DDX_Control(pDX, IDC_ADD_HL7_LOCATION, m_btnAdd);
	DDX_Control(pDX, IDC_REMOVE_HL7_LOCATION, m_btnRemove);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CHL7ConfigLocationsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CHL7ConfigLocationsDlg)
	ON_BN_CLICKED(IDC_ADD_HL7_LOCATION, OnAddHl7Location)
	ON_BN_CLICKED(IDC_REMOVE_HL7_LOCATION, OnRemoveHl7Location)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHL7ConfigLocationsDlg message handlers

BOOL CHL7ConfigLocationsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// (j.jones 2008-05-08 09:38) - PLID 29953 - added nxiconbuttons for modernization
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRemove.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_pLocationList = BindNxDataList2Ctrl(this, IDC_LOCATION_MAP, GetRemoteData(), false);

		IColumnSettingsPtr pCol = m_pLocationList->GetColumn(lcLocationID);
		pCol->PutComboSource("SELECT ID, Name FROM LocationsT "
			"WHERE (Active = 1 AND (TypeID = 1 OR TypeID = 2)) OR ID IN (SELECT LocationID FROM HL7LocationLinkT) "
			"UNION SELECT -1, '{Use Practice Default}' "
			"ORDER BY Name");

		//Validate
		// (j.jones 2010-05-12 09:26) - PLID 36527 - changed the way this dialog loads
		// so we can cache data for auditing		
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name, ThirdPartyID, LocationID "
			"FROM HL7SettingsT "
			"LEFT JOIN HL7LocationLinkT ON HL7SettingsT.ID = HL7LocationLinkT.HL7GroupID "
			"WHERE HL7SettingsT.ID = {INT}", m_nHL7GroupID);
		if(rs->eof) {
			ThrowNxException("Invalid ID passed to CHL7ConfigLocationsDlg!");
		}
		else {
			//cache the group name
			m_strHL7GroupName = VarString(rs->Fields->Item["Name"]->Value, "");

			while(!rs->eof) {

				//if there are no entries in HL7LocationLinkT, ThirdPartyID will be NULL
				_variant_t varThirdPartyID = rs->Fields->Item["ThirdPartyID"]->Value;
				if(varThirdPartyID.vt != VT_NULL) {
					CString strThirdPartyID = VarString(varThirdPartyID);
					long nLocationID = VarLong(rs->Fields->Item["LocationID"]->Value, -1);

					//add to the list
					IRowSettingsPtr pRow = m_pLocationList->GetNewRow();
					pRow->PutValue(lcThirdPartyID, _bstr_t(strThirdPartyID));
					pRow->PutValue(lcLocationID, nLocationID);
					m_pLocationList->AddRowAtEnd(pRow, NULL);

					//store in our array and map
					m_arystrOldCodes.Add(strThirdPartyID);
					m_mapOldCodesToLocationIDs.SetAt(strThirdPartyID, nLocationID);
				}

				rs->MoveNext();
			}
		}
		rs->Close();

		GetDlgItem(IDC_ADD_HL7_LOCATION)->EnableWindow(TRUE);
		GetDlgItem(IDC_REMOVE_HL7_LOCATION)->EnableWindow(m_pLocationList->CurSel == NULL ? FALSE : TRUE);

		return TRUE;

	}NxCatchAll("Error in CHL7ConfigLocationsDlg::OnInitDialog()");

	//We failed to load, so abort!
	OnCancel();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CHL7ConfigLocationsDlg::OnAddHl7Location() 
{
	try {
		BOOL bDone = FALSE;
		while(!bDone) {
			CString strCode;
			if(InputBoxLimited("Enter the location code being imported: ", strCode, "", 255, false, false, NULL) == IDOK) {
				//Did they enter a code?
				if(!strCode.IsEmpty()) {
					//Is it in the list already?
					if(m_pLocationList->FindByColumn(0, _bstr_t(strCode), NULL, VARIANT_FALSE)) {
						//Yup, let them know their mistake.
						MsgBox("The code you entered is already mapped to a Practice location.  Please enter a new code, or if you wish to change the mapping for this code, click in the 'Practice Location' column to select a different location.");
					}
					else {
						//Nope.  Let's add it.
						IRowSettingsPtr pRow = m_pLocationList->GetNewRow();
						pRow->PutValue(lcThirdPartyID, _bstr_t(strCode));
						pRow->PutValue(lcLocationID, (long)-1);
						m_pLocationList->AddRowAtEnd(pRow, NULL);
						m_pLocationList->CurSel = pRow;
						GetDlgItem(IDC_REMOVE_HL7_LOCATION)->EnableWindow(TRUE);
						m_pLocationList->StartEditing(pRow, lcLocationID);
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
	}NxCatchAll("Error in CHL7ConfigLocationsDlg::OnAddHl7Location()");
}

void CHL7ConfigLocationsDlg::OnRemoveHl7Location() 
{
	try {
		if(m_pLocationList->CurSel != NULL) {
			m_pLocationList->RemoveRow(m_pLocationList->CurSel);
		}
	}NxCatchAll("Error in CHL7ConfigLocationsDlg::OnRemoveHl7Location()");
}

void CHL7ConfigLocationsDlg::OnOK() 
{
	long nAuditTransactionID = -1;

	try {
		
		//Let's save, all in one batch.
		CString strSql = BeginSqlBatch();

		//first delete what no longer exists
		for(int i=m_arystrOldCodes.GetSize()-1; i>=0; i--) {
			CString strCode = m_arystrOldCodes.GetAt(i);
			if(m_pLocationList->FindByColumn(0, _bstr_t(strCode), NULL, VARIANT_FALSE) == NULL) {
				//not found, remove it
				//the collate ensures a case-sensitive match
				AddStatementToSqlBatch(strSql, "DELETE FROM HL7LocationLinkT "
					"WHERE HL7GroupID = %li AND ThirdPartyID COLLATE Latin1_General_CS_AS = '%s'", m_nHL7GroupID, _Q(strCode));
				m_arystrOldCodes.RemoveAt(i);
				m_mapOldCodesToLocationIDs.RemoveKey(strCode);

				// (j.jones 2010-05-12 10:24) - PLID 36527 - audit this
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}
				CString strOld, strNew;
				strOld.Format("Location Code '%s' (HL7 Group '%s')", strCode, m_strHL7GroupName);
				strNew.Format("<Code Removed>");
				AuditEvent(-1, "", nAuditTransactionID, aeiHL7LocationLink, m_nHL7GroupID, strOld, strNew, aepLow, aetDeleted);
			}
		}

		//now add/update
		IRowSettingsPtr pRow = m_pLocationList->GetFirstRow();
		while(pRow) {
			CString strThirdPartyID = VarString(pRow->GetValue(lcThirdPartyID), "");
			long nLocationID = VarLong(pRow->GetValue(lcLocationID),-1);
			CString strLocationID;
			if(nLocationID == -1) {
				strLocationID = "NULL";
			}
			else {
				strLocationID.Format("%li", nLocationID);
			}

			//is this setting new?
			BOOL bFound = FALSE;
			for(int i=0;i<m_arystrOldCodes.GetSize() && !bFound;i++) {
				if(m_arystrOldCodes.GetAt(i) == strThirdPartyID) {
					bFound = TRUE;
				}
			}

			if(bFound) {
				//has this setting changed?
				long nOldLocationID = -1;
				m_mapOldCodesToLocationIDs.Lookup(strThirdPartyID, nOldLocationID);
				if(nOldLocationID != nLocationID) {
					//the collate ensures a case-sensitive match
					AddStatementToSqlBatch(strSql, "UPDATE HL7LocationLinkT SET LocationID = %s "
						"WHERE HL7GroupID = %li AND ThirdPartyID COLLATE Latin1_General_CS_AS = '%s'", strLocationID, m_nHL7GroupID, _Q(strThirdPartyID));

					// (j.jones 2010-05-12 10:24) - PLID 36527 - audit this
					if(nAuditTransactionID == -1) {
						nAuditTransactionID = BeginAuditTransaction();
					}
					CString strOld, strNew;
					CString strOldLocationName, strNewLocationName;
					if(nOldLocationID == -1) {
						strOldLocationName = "<Use Practice Default>";
					}
					else {
						_RecordsetPtr rsLocation = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", nOldLocationID);
						if(!rsLocation->eof) {
							strOldLocationName = VarString(rsLocation->Fields->Item["Name"]->Value);
						}
						rsLocation->Close();
					}
					if(strLocationID == -1) {
						strNewLocationName = "<Use Practice Default>";
					}
					else {
						_RecordsetPtr rsLocation = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", nLocationID);
						if(!rsLocation->eof) {
							strNewLocationName = VarString(rsLocation->Fields->Item["Name"]->Value);
						}
						rsLocation->Close();
					}

					strOld.Format("Location Code '%s' (HL7 Group '%s')", strThirdPartyID, m_strHL7GroupName);
					strNew.Format("Link changed from %s to %s", strOldLocationName, strNewLocationName);
					AuditEvent(-1, "", nAuditTransactionID, aeiHL7LocationLink, m_nHL7GroupID, strOld, strNew, aepLow, aetChanged);
				}
			}
			else {
				//create new entry			
				AddStatementToSqlBatch(strSql, "INSERT INTO HL7LocationLinkT (HL7GroupID, ThirdPartyID, LocationID) "
					"VALUES (%li, '%s', %s)", m_nHL7GroupID, _Q(strThirdPartyID), strLocationID);

				// (j.jones 2010-05-12 10:24) - PLID 36527 - audit this
				if(nAuditTransactionID == -1) {
					nAuditTransactionID = BeginAuditTransaction();
				}

				CString strLocationName;
				if(nLocationID == -1) {
					strLocationName = "<Use Practice Default>";
				}
				else {
					_RecordsetPtr rsLocation = CreateParamRecordset("SELECT Name FROM LocationsT WHERE ID = {INT}", nLocationID);
					if(!rsLocation->eof) {
						strLocationName = VarString(rsLocation->Fields->Item["Name"]->Value);
					}
					rsLocation->Close();
				}

				CString strOld, strNew;
				strOld.Format("Location Code '%s' (HL7 Group '%s')", strThirdPartyID, m_strHL7GroupName);
				strNew.Format("Linked to %s", strLocationName);
				AuditEvent(-1, "", nAuditTransactionID, aeiHL7LocationLink, m_nHL7GroupID, strOld, strNew, aepLow, aetCreated);
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

	}NxCatchAllCall("Error in CHL7ConfigLocationsDlg::OnOK()",
		if(nAuditTransactionID != -1) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
	)
}

BEGIN_EVENTSINK_MAP(CHL7ConfigLocationsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CHL7ConfigLocationsDlg)
	ON_EVENT(CHL7ConfigLocationsDlg, IDC_LOCATION_MAP, 2 /* SelChanged */, OnSelChangedLocationMap, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CHL7ConfigLocationsDlg, IDC_LOCATION_MAP, 7 /* RButtonUp */, OnRButtonUpLocationMap, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CHL7ConfigLocationsDlg, IDC_LOCATION_MAP, 9, CHL7ConfigLocationsDlg::OnEditingFinishingLocationMap, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

void CHL7ConfigLocationsDlg::OnSelChangedLocationMap(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	if(lpNewSel == NULL) {
		GetDlgItem(IDC_REMOVE_HL7_LOCATION)->EnableWindow(FALSE);
	}
	else {
		GetDlgItem(IDC_REMOVE_HL7_LOCATION)->EnableWindow(TRUE);
	}
}

void CHL7ConfigLocationsDlg::OnRButtonUpLocationMap(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if(lpRow == NULL) return;
		IRowSettingsPtr pRow(lpRow);
		m_pLocationList->CurSel = pRow;
		GetDlgItem(IDC_REMOVE_HL7_LOCATION)->EnableWindow(TRUE);

		CMenu mnu;
		if (mnu.CreatePopupMenu()) {
			//Add our item.
			mnu.InsertMenu(-1, MF_BYPOSITION, 1, "Remove");
			CPoint pt;
			GetCursorPos(&pt);
			int nResult = mnu.TrackPopupMenu(TPM_NONOTIFY|TPM_RETURNCMD, pt.x, pt.y, this);
			//Did they select our item?
			if (nResult == 1) {
				OnRemoveHl7Location();
			} else {
				//They didn't choose anything, so we don't need to do anything?
				ASSERT(nResult == 0);
				return;
			}
		} else {
			//Wha??
			ASSERT(FALSE);
		}
	}NxCatchAll("Error in CHL7ConfigLocationsDlg::OnRButtonUpLocationMap()");
}

// (j.jones 2010-05-12 10:20) - PLID 36527 - this dialog never validated the data entered before
void CHL7ConfigLocationsDlg::OnEditingFinishingLocationMap(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {

		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		if(nCol == lcThirdPartyID) {

			CString strCode = strUserEntered;
			if(strCode.IsEmpty()) {
				AfxMessageBox("You cannot enter a blank code.");
				*pbContinue = FALSE;
				*pbCommit = FALSE;
				return;
			}
			else {
				//see if it already exists
				IRowSettingsPtr pFindRow = m_pLocationList->GetFirstRow();
				while(pFindRow) {

					//skip this row
					if(pFindRow == pRow) {
						pFindRow = pFindRow->GetNextRow();
						continue;
					}

					CString strFindThirdPartyID = VarString(pFindRow->GetValue(lcThirdPartyID), "");

					//case sensitive match
					if(strFindThirdPartyID == strCode) {
						AfxMessageBox("The code you entered already exists in the list.");
						*pbContinue = FALSE;
						*pbCommit = FALSE;
						return;
					}

					pFindRow = pFindRow->GetNextRow();
				}
			}
		}
	
	}NxCatchAll("Error in CHL7ConfigLocationsDlg::OnEditingFinishingLocationMap()");
}
