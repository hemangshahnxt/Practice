// EmrCollectionSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EmrCollectionSetupDlg.h"
#include "EMRTopic.h"
#include "AuditTrail.h"
#include "MsgBox.h"
#include "GlobalDataUtils.h"

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum EEmrCollectionListColumns {
	eclcModified,
	eclcID,
	eclcMenuOrder,
	eclcCollection,
	eclcAppTyp,// (s.dhole 2010-02-11 16:38) - PLID 37112 Workflow change from room manager -> EMR for doctors
	eclcBoldType, // (j.gruber 2010-04-23 15:30) - PLID 38338 - bold type
	eclcInactive,
};

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CEmrCollectionSetupDlg dialog

CEmrCollectionSetupDlg::CEmrCollectionSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrCollectionSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmrCollectionSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CEmrCollectionSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrCollectionSetupDlg)
	DDX_Control(pDX, IDC_DATA_UP, m_btnDataUp);
	DDX_Control(pDX, IDC_DATA_DOWN, m_btnDataDown);
	DDX_Control(pDX, IDC_ADD_BTN, m_btnAdd);
	DDX_Control(pDX, IDC_RENAME_BTN, m_btnRename);
	DDX_Control(pDX, IDC_DELETE_BTN, m_btnDelete);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrCollectionSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrCollectionSetupDlg)
	ON_BN_CLICKED(IDC_DATA_UP, OnDataUpBtn)
	ON_BN_CLICKED(IDC_DATA_DOWN, OnDataDownBtn)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_ADD_BTN, OnAddBtn)
	ON_BN_CLICKED(IDC_RENAME_BTN, OnRenameBtn)
	ON_BN_CLICKED(IDC_DELETE_BTN, OnDeleteBtn)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CEmrCollectionSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrCollectionSetupDlg)
	ON_EVENT(CEmrCollectionSetupDlg, IDC_EMRCOLLECTION_LIST, 10 /* EditingFinished */, OnEditingFinishedEmrcollectionList, VTS_I4 VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CEmrCollectionSetupDlg, IDC_EMRCOLLECTION_LIST, 6 /* RButtonDown */, OnRButtonDownEmrcollectionList, VTS_I4 VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CEmrCollectionSetupDlg, IDC_EMRCOLLECTION_LIST, 2 /* SelChanged */, OnSelChangedEmrcollectionList, VTS_I4)
	ON_EVENT(CEmrCollectionSetupDlg, IDC_EMRCOLLECTION_LIST, 9 /* EditingFinishing */, OnEditingFinishingEmrcollectionList, VTS_I4 VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


/////////////////////////////////////////////////////////////////////////////
// CEmrCollectionSetupDlg message handlers

BOOL CEmrCollectionSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// Set the button images
		m_btnDataUp.AutoSet(NXB_UP);
		m_btnDataDown.AutoSet(NXB_DOWN);
		m_btnAdd.AutoSet(NXB_NEW);
		m_btnRename.AutoSet(NXB_MODIFY);
		m_btnDelete.AutoSet(NXB_DELETE);
		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// Bind the datalist but don't load
		m_pdlEmrCollectionList = BindNxDataListCtrl(this, IDC_EMRCOLLECTION_LIST, GetRemoteData(), false);

		// (j.gruber 2010-06-04 15:14) - PLID 38935 - hide BOLD type column if you aren't licensed
		if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBold, CLicense::cflrSilent)) {
			m_pdlEmrCollectionList->GetColumn(eclcBoldType)->PutColumnStyle(csVisible|csFixedWidth);
			m_pdlEmrCollectionList->GetColumn(eclcBoldType)->PutStoredWidth(0);
		}

		// Load the whole screen
		Load();

		// Set the focus to the datalist
		GetDlgItem(IDC_EMRCOLLECTION_LIST)->SetFocus();

		// We're good to go, return FALSE because we set the focus ourselves
		return FALSE;

	} NxCatchAllCall("CEmrCollectionSetupDlg::OnInitDialog", {
		// Fail
		EndDialog(IDCANCEL);
		return FALSE;
	});
}

void CEmrCollectionSetupDlg::Load()
{
	// Clear the "deleted" array
	m_arydwDeletedCollectionIDs.RemoveAll();	

	// Requery the collection list
	m_pdlEmrCollectionList->Requery();

	// Reflect the proper "enabled" state for each button
	ReflectEnabledState();
}

// (z.manning, 02/23/2007) - This struct is used in CEmrCollectionSetupDlg::ValidateAndSave to store info
// to audit any EMN templates that get deleted.
struct EmnTemplateAuditInfo
{
	long nID;
	CString strName;
};

BOOL CEmrCollectionSetupDlg::ValidateAndSave()
{
	// Renumber everything prior to saving
	RenumberCollectionMenuOrders();	
	
	CArray<EmnTemplateAuditInfo,EmnTemplateAuditInfo&> aryDeletedTemplates;

	// We'll fill this string with a series of statements to be executed at the end within a transaction
	CString strSql = BeginSqlBatch();
	{
		// Delete any that don't exist anymore
		if(m_arydwDeletedCollectionIDs.GetSize() > 0)
		{			
			// Create a comma-separated list of deleted IDs.
			CString strIDs;
			for (long i=0; i<m_arydwDeletedCollectionIDs.GetSize(); i++) {
				CString strID;
				long nID = m_arydwDeletedCollectionIDs.GetAt(i);
				if (nID != -1) {
					strID.Format("%li", nID);
					strIDs += strID + ",";
				}
			}
			// If there was anything in the list, add a DELETE statement to our batch
			// (m.cable 6/21/2004 09:07) - I added the two statements deleting from EMRTemplateDetailsT and
			// EMRTemplateTopicsT.  Otherwise there was a foreign key constraint violation on the TemplateID
			// (m.cable 6/25/2004 09:32) - PLID 13099 - If the EMR has been deleted, then update the CollectionID
			// in EMRMasterT to be Null
			//TES 10/12/2004 - I'm allowing EmrTemplateT to be actually deleted here (instead of marked "Deleted"), because
			//if it had ever actually been used, the user would have been prevented from deleting the collection.
			// (c.haag 2004-12-09 17:31) - PLID 14884 - Make sure no details have the SourceTemplateID of the template we are deleting
			if (!strIDs.IsEmpty()) {
				strIDs.Delete(strIDs.GetLength() - 1, 1);

				// (z.manning, 02/22/2007) - PLID 23899 - We need to find all templates in any deleted collection
				// so that we can audit their deletion.
				_RecordsetPtr prsDeletedTemplateIDs = CreateRecordset(
					"SELECT ID, Name FROM EmrTemplateT "
					"WHERE CollectionID IN (%s) ", strIDs);
				while(!prsDeletedTemplateIDs->eof) {
					EmnTemplateAuditInfo etai;
					etai.nID = AdoFldLong(prsDeletedTemplateIDs, "ID");
					etai.strName = AdoFldString(prsDeletedTemplateIDs, "Name", "");
					aryDeletedTemplates.Add(etai);
					prsDeletedTemplateIDs->MoveNext();
				}

				// (j.dinatale 2012-09-10 10:15) - PLID 52551 - delete from our OMR tables
				AddStatementToSqlBatch(strSql, "DELETE OMRFormDetailT FROM OMRFormDetailT INNER JOIN OMRFormT ON OMRFormDetailT.OMRFormID = OMRFormT.ID WHERE OMRFormT.EMRTemplateID IN (SELECT EMRTemplateT.ID FROM EMRTemplateT WHERE EMRTemplateT.CollectionID IN (%s))", strIDs);
				AddStatementToSqlBatch(strSql, "DELETE FROM OMRFormT WHERE OMRFormT.EMRTemplateID IN (SELECT EMRTemplateT.ID FROM EMRTemplateT WHERE EMRTemplateT.CollectionID IN (%s))", strIDs);
				
				// (z.manning, 09/19/2006) - Back in 2004, we made it so that EMN templates can't be deleted
				// since EMNs (and details and potentially more things in the future) track the template ID
				// where they came from.
				//(e.lally 2011-05-05) PLID 43481 - Set to not visible in the NexWeb display setup too for clarity
				AddStatementToSqlBatch(strSql, "UPDATE NexWebDisplayT SET Visible =0 WHERE EmrTemplateID IN (SELECT ID FROM EMRTemplateT WHERE CollectionID IN (%s))", strIDs);
				AddStatementToSqlBatch(strSql, "UPDATE EmrActionsT SET Deleted = 1 WHERE DestType = %i AND DestID IN (SELECT ID FROM EMRTemplateT WHERE CollectionID IN (%s))", eaoMint, strIDs);
				AddStatementToSqlBatch(strSql, "UPDATE EmrActionsT SET Deleted = 1 WHERE DestType = %i AND DestID IN (SELECT ID FROM EMRTemplateT WHERE CollectionID IN (%s))", eaoMintItems, strIDs);
				AddStatementToSqlBatch(strSql, "UPDATE EMRTemplateT SET Deleted = 1, CollectionID = NULL WHERE CollectionID IN (%s)", strIDs);
				// (a.walling 2008-06-25 17:51) - PLID 30515 - Fail if any templates are being modified
				// (j.armen 2013-05-14 11:43) - PLID 56683 - EMN Template Access Refactoring
				AddStatementToSqlBatch(strSql, 
					"IF EXISTS(\r\n"
					"	SELECT 1\r\n"
					"	FROM EMNTemplateAccessT WITH(UPDLOCK, HOLDLOCK)\r\n"
					"	WHERE EmnID IN (SELECT ID FROM EMRTemplateT WHERE CollectionID IN (%s)))\r\n"
					"BEGIN\r\n"
					"	RAISERROR('Template cannot be deleted; it is being modified by another user.', 16, 43)\r\n"
					"	ROLLBACK TRAN\r\n"
					"	RETURN\r\n"
					"END", strIDs);
				// (c.haag 2009-02-23 14:51) - PLID 32761 - Also delete from default procedures
				AddStatementToSqlBatch(strSql, "DELETE FROM EmrDefaultTemplateProceduresT WHERE TemplateID IN (SELECT ID FROM EMRDefaultTemplatesT WHERE EmrCollectionID IN (%s))", strIDs);
				AddStatementToSqlBatch(strSql, "DELETE FROM EMRDefaultTemplatesT WHERE EmrCollectionID IN (%s)", strIDs);
				AddStatementToSqlBatch(strSql, "DELETE FROM EMRCollectionTemplateT WHERE CollectionID IN (%s)", strIDs);
				AddStatementToSqlBatch(strSql, "DELETE FROM EMRCollectionT WHERE ID IN (%s)", strIDs);
			}
		}
		
		// Loop through the list, adding new ones and updating existing ones
		{
			// Iterate the list of rows
			long p = m_pdlEmrCollectionList->GetFirstRowEnum();
			while (p) {
				// Get this row (and advance the iteration to the next row)
				LPDISPATCH lpDisp;
				m_pdlEmrCollectionList->GetNextRowEnum(&p, &lpDisp);
				if (lpDisp) {
					// If this row was modified, add to the sql batch
					IRowSettingsPtr pRow(lpDisp);
					lpDisp->Release();
					if (VarBool(pRow->GetValue(eclcModified))) {
						// This row has been modified, so create the statement to either create the record, or update 
						// the values in the existing record.
						long nID = VarLong(pRow->GetValue(eclcID));
						_variant_t varMenuOrder = pRow->GetValue(eclcMenuOrder);
						_variant_t varAppTypeID = pRow->GetValue(eclcAppTyp); // (s.dhole 2010-02-11 16:38) - PLID 37112 Workflow change from room manager -> EMR for doctors
						// (j.gruber 2010-04-23 15:34) - PLID 38338 - Bold Type
						_variant_t varBoldTypeID = pRow->GetValue(eclcBoldType);
						BOOL bInactive = VarBool(pRow->GetValue(eclcInactive));
						// (s.dhole 2010-02-11 16:38) - PLID 37112 Workflow change from room manager -> EMR for doctors
						if (nID == -1) {
							// It's new, so create it
							// (j.gruber 2010-04-23 15:32) - PLID 38338 - Bold Type
							AddStatementToSqlBatch(strSql, "INSERT INTO EMRCollectionT (Name, MenuOrder,AptTypeID, Inactive, BoldTypeID) VALUES ('%s', %s,%s , %i, %s)", 
								_Q(VarString(pRow->GetValue(eclcCollection))), varMenuOrder.vt == VT_I4 ? AsString(varMenuOrder) : "NULL",varAppTypeID.vt == VT_I4 ? (AsString(varAppTypeID) == "0" ? "NULL" :AsString(varAppTypeID) )   : "NULL" ,  
								bInactive ? 1 : 0,
								varBoldTypeID.vt == VT_I4 ? VarLong(varBoldTypeID, -1) == -1 ? "NULL" : AsString(varBoldTypeID) : "NULL");
						} else {
							// It already exists, so update it
							// (j.gruber 2010-04-23 15:32) - PLID 38338 - Bold Type
							AddStatementToSqlBatch(strSql, "UPDATE EMRCollectionT SET Name = '%s', MenuOrder = %s, AptTypeID =  %s, BoldTypeID = %s, Inactive = %i WHERE ID = %li", 
								_Q(VarString(pRow->GetValue(eclcCollection))), varMenuOrder.vt == VT_I4 ? AsString(varMenuOrder): "NULL",varAppTypeID.vt == VT_I4 ? (AsString(varAppTypeID) == "0" ? "NULL" :AsString(varAppTypeID)) : "NULL", 
								varBoldTypeID.vt == VT_I4 ? VarLong(varBoldTypeID, -1) == -1 ? "NULL" : AsString(varBoldTypeID) : "NULL",
								bInactive ? 1 : 0, nID);
						}
					}
				}
			}
		}
	}

	// (a.walling 2012-04-10 17:16) - PLID 49564 - Ensure we don't show a blank CMsgBox in debug mode when we don't actually have anything to update

	// If we made it here we successfully validated and generated our save string, so execute it
	if (!strSql.IsEmpty()) {

#ifdef _DEBUG
		// In debug mode give the user a chance to review the sql batch to be 
		// executed and optionally cancel the save.
		//(e.lally 2008-04-10) - changed this to an OK|CANCEL CMsgBox
		CMsgBox dlg(this);
		dlg.msg = strSql;
		dlg.m_bAllowCancel = TRUE;
		UINT result = dlg.DoModal();
		if (result != IDOK) { 
			return FALSE; 
		}
#endif

		// Run the statement
		ExecuteSqlBatch(strSql);
		CClient::RefreshTable(NetUtils::EMRCollectionT);

		// (z.manning, 02/23/2007) - Now audit any EMN templates that were deleted as a result of
		// deleting collections.
		if(aryDeletedTemplates.GetSize() > 0) {
			long nAuditID = BeginNewAuditEvent();
			for(int i = 0; i < aryDeletedTemplates.GetSize(); i++) {
				AuditEvent(-1, "", nAuditID, aeiEMNTemplateDeleted, aryDeletedTemplates.GetAt(i).nID, aryDeletedTemplates.GetAt(i).strName, "<Deleted>", aepHigh, aetDeleted);
			}
		}
	} else {
		// No statements to run, which means nothing changed so there's nothing to save
		ASSERT(m_arydwDeletedCollectionIDs.GetSize() == 0);
	}

	// Now that we're finished saving we have to mark everything on the dialog as 
	// being clean again (unmodified) including setting all menuorders and IDs and 
	// clearing the m_arydwDeletedCollectionIDs array of deleted items.
	
	// NOTE: A handy shortcut to doing all the above manually is to just call Load.
	Load();

	// We validated successfully (and saved as well) so we return TRUE.
	return TRUE;
}

void CEmrCollectionSetupDlg::OnOK() 
{
	try {
		if (ValidateAndSave()) {
			// We're done
			CNxDialog::OnOK();
		} else {
			// It failed validation and the user has already been notified so do nothing
		}
	} NxCatchAll("CEmrCollectionSetupDlg::OnOK");
}

void SwapRows(_DNxDataList *lpdl, long nRowIndexFrom, long nRowIndexTo, short nColToRetain, BOOL bAutoSelectNew)
{
	// Validate
	_DNxDataListPtr pdl(lpdl);
	long nRowCount = pdl->GetRowCount();
	if (pdl != NULL && nRowIndexFrom != sriNoRow && nRowIndexTo != sriNoRow && nRowIndexFrom < nRowCount && nRowIndexTo < nRowCount) {
		// Exchange the nColToRetain column's values
		if (nColToRetain != -1) {
			_variant_t var = pdl->GetValue(nRowIndexFrom, nColToRetain);
			pdl->PutValue(nRowIndexFrom, nColToRetain, pdl->GetValue(nRowIndexTo, nColToRetain));
			pdl->PutValue(nRowIndexTo, nColToRetain, var);
		}

		// Shift the FROM row to be positioned where the TO row was, and vice versa
		{
			// Move the FROM row first (but get the TO row so we can reference it later)
			IRowSettingsPtr pRowFrom = pdl->GetRow(nRowIndexFrom);
			IRowSettingsPtr pRowTo = pdl->GetRow(nRowIndexTo);
			pdl->RemoveRow(nRowIndexFrom);
			pdl->InsertRow(pRowFrom, nRowIndexTo);
			// Now if the new index of the TO row hasn't automatically become what the FROM row WAS, then we need to move it too.
			long nNewToIndex = pRowTo->GetIndex();
			if (nNewToIndex != nRowIndexFrom) {
				pdl->RemoveRow(nNewToIndex);
				pdl->InsertRow(pRowTo, nRowIndexFrom);
			}
		}

		// Auto-select the original FROM row (if desired)
		if (bAutoSelectNew) {
			pdl->PutCurSel(nRowIndexTo);
		}
	}
}

void CEmrCollectionSetupDlg::RenumberCollectionMenuOrders()
{
	long nMenuOrder = 1;

	long p = m_pdlEmrCollectionList->GetFirstRowEnum();
	while (p) {
		// Get this row (and advance the iteration to the next row)
		LPDISPATCH lpDisp;
		m_pdlEmrCollectionList->GetNextRowEnum(&p, &lpDisp);
		if (lpDisp) {
			// Change this row's menu order if it's not already correct
			IRowSettingsPtr pRow(lpDisp);
			lpDisp->Release();
			_variant_t varMenuOrder = pRow->GetValue(eclcMenuOrder);
			if (varMenuOrder.vt == VT_NULL || VarLong(varMenuOrder) != nMenuOrder) {
				pRow->PutValue(eclcMenuOrder, nMenuOrder);
				pRow->PutValue(eclcModified, g_cvarTrue);
			}
			// Increment the menu order
			nMenuOrder++;
		}
	}
}

void CEmrCollectionSetupDlg::OnDataUpBtn()
{
	try {
		long nCurSel = m_pdlEmrCollectionList->GetCurSel();
		if (nCurSel != sriNoRow && nCurSel > 0) {
			// Perform the swap in the standard way
			long nNewSel = nCurSel - 1;
			SwapRows(m_pdlEmrCollectionList, nCurSel, nNewSel, eclcMenuOrder, TRUE);
			m_pdlEmrCollectionList->PutValue(nCurSel, eclcModified, _variant_t(VARIANT_TRUE, VT_BOOL));
			m_pdlEmrCollectionList->PutValue(nNewSel, eclcModified, _variant_t(VARIANT_TRUE, VT_BOOL));
			
			// Renumber everything
			RenumberCollectionMenuOrders();

			// Reflect the proper "enabled" state for each button
			ReflectEnabledState();
		}
	} NxCatchAll("CEmrCollectionSetupDlg::OnDataUpBtn");
}

void CEmrCollectionSetupDlg::OnDataDownBtn()
{
	try {
		long nCurSel = m_pdlEmrCollectionList->GetCurSel();
		if (nCurSel != sriNoRow && nCurSel < m_pdlEmrCollectionList->GetRowCount() - 1) {
			// Perform the swap in the standard way
			long nNewSel = nCurSel + 1;
			SwapRows(m_pdlEmrCollectionList, nCurSel, nNewSel, eclcMenuOrder, TRUE);
			m_pdlEmrCollectionList->PutValue(nCurSel, eclcModified, _variant_t(VARIANT_TRUE, VT_BOOL));
			m_pdlEmrCollectionList->PutValue(nNewSel, eclcModified, _variant_t(VARIANT_TRUE, VT_BOOL));
			
			// Renumber everything
			RenumberCollectionMenuOrders();

			// Reflect the proper "enabled" state for each button
			ReflectEnabledState();
		}
	} NxCatchAll("CEmrCollectionSetupDlg::OnDataDownBtn");
}

void CEmrCollectionSetupDlg::OnEditingFinishedEmrcollectionList(long nRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if (bCommit) {
			// (j.gruber 2010-04-23 15:45) - PLID 38338 - BOLD type
			if (nRow != sriNoRow && (nCol == eclcMenuOrder || nCol == eclcCollection || nCol == eclcAppTyp || nCol == eclcBoldType || nCol == eclcInactive)) {
				BOOL bChanged = FALSE;
				if (varOldValue.vt == VT_BSTR && varNewValue.vt == VT_BSTR) {
					// Strings are a special case, we should ignore trailing whitespace, but NOT ignore case (upper vs lower)
					CString strOld = VarString(varOldValue);
					CString strNew = VarString(varNewValue);
					strOld.TrimRight();
					strNew.TrimRight();
					if(strNew != strOld) {
						bChanged = TRUE;
					} else {
						bChanged = FALSE;
					}
				} else { 
					// Any other combination of data types, use the old VariantCompareLenient()
					if (VariantCompareLenient(&_variant_t(varOldValue), &_variant_t(varNewValue)) != VARCMP_EQ) {
						bChanged = TRUE;
					} else {
						bChanged = FALSE;
					}
				}

				// Now we know whether it was changed or not, so behave accordingly
				if (bChanged) {
					// A change is being committed so flag this row as having changed
					m_pdlEmrCollectionList->PutValue(nRow, eclcModified, _variant_t(VARIANT_TRUE, VT_BOOL));
				}
			} else {
				// We have the datalist set up in resources so that the user can only edit the menu order and 
				// collection columns.  Plus, the datalist doesn't have an interface for editing a row that 
				// doesn't exist (how could it), so it should be completely impossible to get to this branch.
				ASSERT(FALSE);
			}
		}
	} NxCatchAll("CEmrCollectionSetupDlg::OnEditingFinishedEmrcollectionList");
}

void CEmrCollectionSetupDlg::OnRButtonDownEmrcollectionList(long nRow, short nCol, long x, long y, long nFlags) 
{
	// Take focus
	GetDlgItem(IDC_EMRCOLLECTION_LIST)->SetFocus();

	// Change the selection to be the one the user right-clicked on
	m_pdlEmrCollectionList->PutCurSel(nRow);
	
	// Reflect the proper "enabled" state for each button
	ReflectEnabledState();

	// (b.cardillo 2004-06-07 15:54) - This is a workaround for a bug in the datalist where it doesn't 
	// properly invalidate itself to move the focus rect when the cursel is changed by the client app 
	// (in this case us), so we invalidate for it.
	GetDlgItem(IDC_EMRCOLLECTION_LIST)->Invalidate(FALSE);
}

void CEmrCollectionSetupDlg::ReflectEnabledState()
{
	long nCurSel = m_pdlEmrCollectionList->GetCurSel();
	BOOL bRowSelected = (nCurSel != sriNoRow);
	m_btnRename.EnableWindow(bRowSelected);
	m_btnDelete.EnableWindow(bRowSelected);
	m_btnDataUp.EnableWindow(bRowSelected && (nCurSel > 0));
	m_btnDataDown.EnableWindow(bRowSelected && (nCurSel < (m_pdlEmrCollectionList->GetRowCount() - 1)));
}

void CEmrCollectionSetupDlg::OnSelChangedEmrcollectionList(long nNewSel) 
{
	try {
		ReflectEnabledState();
	} NxCatchAll("CEmrCollectionSetupDlg::OnSelChangedEmrcollectionList");
}

void CEmrCollectionSetupDlg::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	try {
		if (pWnd->GetSafeHwnd() && pWnd->GetSafeHwnd() == GetDlgItem(IDC_EMRCOLLECTION_LIST)->GetSafeHwnd()) {
			// Make sure something is selected
			long nCurSel = m_pdlEmrCollectionList->GetCurSel();
			if (nCurSel != sriNoRow) {
				// Create the menu
				CMenu mnu;
				mnu.CreatePopupMenu();
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, IDC_RENAME_BTN, "&Rename");
				mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, IDC_DELETE_BTN, "&Delete");
				CPoint pt = CalcContextMenuPos(pWnd, pos);
				mnu.TrackPopupMenu(TPM_LEFTALIGN, pt.x, pt.y, this);
			}
		}
	} NxCatchAll("CEmrCollectionSetupDlg::OnContextMenu");
}

void CEmrCollectionSetupDlg::OnAddBtn()
{
	try {
		// This is easy, just create a row, fill it and add it at the end of the list
		{
			// First figure out what the new menu order will be
			long nNewMenuOrder;
			{
				long nRowCount = m_pdlEmrCollectionList->GetRowCount();
				if (nRowCount > 0) {
					_variant_t varLastMenuOrder = m_pdlEmrCollectionList->GetValue(nRowCount - 1, eclcMenuOrder);
					if (varLastMenuOrder.vt != VT_NULL) {
						nNewMenuOrder = VarLong(varLastMenuOrder) + 1;
					} else {
						nNewMenuOrder = 0;
					}
				} else {
					nNewMenuOrder = 0;
				}
			}

			CString strNewName;
			long ID = m_pdlEmrCollectionList->GetRowCount() + 1;
			strNewName.Format("[New Collection %li]",ID);
			while(m_pdlEmrCollectionList->FindByColumn(eclcCollection,_bstr_t(strNewName),0,FALSE) != -1) {
				ID++;
				strNewName.Format("[New Collection %li]",ID);
			}

			variant_t varFalse;
			varFalse.vt = VT_BOOL;
			varFalse.boolVal = FALSE;
			
			// Now create and fill the row
			IRowSettingsPtr pRow = m_pdlEmrCollectionList->GetRow(sriGetNewRow);
			pRow->PutValue(eclcModified, g_cvarTrue);
			pRow->PutValue(eclcID, (long)-1);
			pRow->PutValue(eclcMenuOrder, nNewMenuOrder);
			pRow->PutValue(eclcCollection, _bstr_t(strNewName));
			// (j.gruber 2010-04-23 15:28) - PLID 38338 - BOLD Type
			pRow->PutValue(eclcBoldType, g_cvarNull);
			pRow->PutValue(eclcInactive, varFalse);

			// And add it to the list
			long nNewRowIndex = m_pdlEmrCollectionList->InsertRow(pRow, -1);

			// Move to it and start editing the name for the users convenience
			if (nNewRowIndex != sriNoRow) {
				// Set the selection
				m_pdlEmrCollectionList->PutCurSel(nNewRowIndex);
				// Start editing the row
				m_pdlEmrCollectionList->StartEditing(nNewRowIndex, eclcCollection);
			} else {
				// Failure for some reason!
				ThrowNxException("An EMR Collection could not be added!");
			}
		}
	} NxCatchAll("CEmrCollectionSetupDlg::OnAddBtn");
}

void CEmrCollectionSetupDlg::OnRenameBtn()
{
	try {
		long nCurSel = m_pdlEmrCollectionList->GetCurSel();
		if (nCurSel != sriNoRow) {
			m_pdlEmrCollectionList->StartEditing(nCurSel, eclcCollection);
		} else {
			// We shouldn't be able to get here because when no row is selected the buttons are supposed to be disabled
			ASSERT(FALSE);
			MessageBox("There are currently no rows selected on which to perform this operation.  Please select a row and try again.", NULL, MB_OK|MB_ICONEXCLAMATION);
		}
	} NxCatchAll("CEmrCollectionSetupDlg::OnRenameBtn");
}

void CEmrCollectionSetupDlg::OnDeleteBtn()
{
	try {
		long nCurSel = m_pdlEmrCollectionList->GetCurSel();
		if (nCurSel != sriNoRow) {
			// Confirm the delete
			BOOL bProceed;
			{
				// Decide on the type of confirmation required ("are you sure?" or "sorry you can't.")
				BOOL bAllowDelete;
				BOOL bTryToInactivate = FALSE;
				CString strMsg;
				{
					CString strCollectionName = VarString(m_pdlEmrCollectionList->GetValue(nCurSel, eclcCollection));
					long nID = m_pdlEmrCollectionList->GetValue(nCurSel, eclcID);
					if (nID != -1) {
						// (j.gruber 2012-08-31 13:20) - PLID 52285 - check how many tempaltes have OMR Templates (also made param)
						_RecordsetPtr prs = CreateParamRecordset(
							"SELECT "
							"	(SELECT COUNT(*) FROM EMRMasterT WHERE EmrCollectionID = {INT}) AS EmrCount, "
							"	(SELECT COUNT(*) FROM EMRTemplateT WHERE Deleted = 0 AND CollectionID = {INT}) AS TemplateCount, "
							"	(SELECT COUNT(*) FROM EMRTemplateT WHERE ID IN (SELECT EMRTemplateID FROM OMRFormT) AND Deleted = 0 AND CollectionID = {INT}) AS TemplateWithOMRCount", 
							nID, nID, nID);
						FieldsPtr pflds = prs->GetFields();
						long nEmrCount = AdoFldLong(pflds, "EmrCount");
						long nTemplateCount = AdoFldLong(pflds, "TemplateCount");
						long nTempWithOMRs = AdoFldLong(pflds, "TemplateWithOMRCount");
						if (nEmrCount > 0) {
							// This collection is in use, we cannot delete it
							bAllowDelete = FALSE;
							
							//Should we try and inactivate it?
							if(VarBool(m_pdlEmrCollectionList->GetValue(nCurSel, eclcInactive))) {
								//Nope, it's already inactive.
								// The message should just say no
								strMsg.Format(
									"The '%s' EMR Collection is in use by at least one EMR and therefore cannot be deleted.", 
									strCollectionName);
								bTryToInactivate = FALSE;
							}
							else {
								//Yes, we'll ask them.
								strMsg.Format("The '%s' EMR Collection is in use by at least one EMR and therefore cannot be deleted.\n"
									"Would you like to mark it Inactive instead?", strCollectionName);
								bTryToInactivate = TRUE;
							}
						} else {
							// No EMRs are using it so we're allowed to delete it, pending the user's approval
							bAllowDelete = TRUE;
							// We'll give a different message depending on if there are any templates that use it
							if (nTemplateCount > 0) {
								// Tell the user how many templates
								if (nTempWithOMRs > 0) {
									strMsg.Format(
										"The '%s' EMR Collection has %li EMR Templates associated with it, %li of which are associated with OMR forms.  Are you sure "
										"you want to delete the EMR Collection and all its EMR Templates?",
										strCollectionName, nTemplateCount, nTempWithOMRs);
								}
								else {
									strMsg.Format(
										"The '%s' EMR Collection has %li EMR Templates associated with it.  Are you sure "
										"you want to delete the EMR Collection and all its EMR Templates?",
										strCollectionName, nTemplateCount);							
								}
							}else {
								// Just ask if the user wants to delete the collection
								strMsg.Format("Are you sure you want to delete the '%s' EMR Collection?", strCollectionName);
							}
						}
					} else {
						// The collection isn't even in data yet so of course it can be deleted.
						bAllowDelete = TRUE;
						// But we still prompt the user
						strMsg.Format("Are you sure you want to delete the '%s' EMR Collection?", strCollectionName);
					}
				}
				// Based on the kind of confirmation decided on above, prompt the user accordingly.
				if (bAllowDelete) {
					if (MessageBox(strMsg, NULL, MB_YESNO|MB_ICONQUESTION) == IDYES) {
						bProceed = TRUE;
					} else {
						bProceed = FALSE;
					}
				} else {
					if(bTryToInactivate) {
						if(IDYES == MessageBox(strMsg, NULL, MB_YESNO|MB_ICONEXCLAMATION)) {
							m_pdlEmrCollectionList->PutValue(nCurSel, eclcInactive, true);
							m_pdlEmrCollectionList->PutValue(nCurSel, eclcModified, true);
						}
					}
					else {
						MessageBox(strMsg, NULL, MB_OK|MB_ICONEXCLAMATION);
					}
					bProceed = FALSE;
				}
			}
			// Now only go forward if it was decided that we are to do so.
			if (bProceed) {
				// The user said go for it, so go for it
				m_arydwDeletedCollectionIDs.Add(VarLong(m_pdlEmrCollectionList->GetValue(nCurSel, eclcID)));
				m_pdlEmrCollectionList->RemoveRow(nCurSel);
				// Reflect the proper "enabled" state for each button
				ReflectEnabledState();
			}
		} else {
			// We shouldn't be able to get here because when no row is selected the buttons are supposed to be disabled
			ASSERT(FALSE);
			MessageBox("There are currently no rows selected on which to perform this operation.  Please select a row and try again.", NULL, MB_OK|MB_ICONEXCLAMATION);
		}
	} NxCatchAll("CEmrCollectionSetupDlg::OnDeleteBtn");
}

BOOL CEmrCollectionSetupDlg::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message) {
	case WM_KEYDOWN:
		try {
			// If the user hit delete and the delete is allowed right now and the datalist has focus (which 
			// also serves to ensure that the user isn't editing an entry in the datalist right now) then go 
			// ahead and process the OnDeleteBtn() handler and don't let it pass to the normal handler.
			if (pMsg->wParam == VK_DELETE) {
				if (GetDlgItem(IDC_DELETE_BTN)->IsWindowEnabled()) {
					// Make sure the datalist has focus
					HWND hwndFocus = ::GetFocus();
					if (hwndFocus == NULL || hwndFocus == GetDlgItem(IDC_EMRCOLLECTION_LIST)->GetSafeHwnd()) {
						// Process our  handler
						OnDeleteBtn();
						// Don't let the message back out to the normal handler.
						return TRUE;
					}
				}
			}
		} NxCatchAll("CEmrCollectionSetupDlg::PreTranslateMessage:WM_KEYDOWN");
		break;
	default:
		break;
	}

	return CNxDialog::PreTranslateMessage(pMsg);
}

bool CEmrCollectionSetupDlg::IsCollectionNameValid(CString strCollectionNameToValidate, CString strOldCollectionName /* ="" */)
{
	//check to make sure the name isn't blank
	if(strCollectionNameToValidate == "") {
			MessageBox("You cannot enter a collection with a blank name.");
			return false;
	}

	// if the new name and the old name are the same, then the name is valid
	if(strCollectionNameToValidate.CompareNoCase(strOldCollectionName) == 0){
		return true;
	}
	
	//check to make sure the group name isn't duplicated
	long pCurRowEnum = m_pdlEmrCollectionList->GetFirstRowEnum();
	while(pCurRowEnum != 0){
		IRowSettingsPtr pRow;
		{
			IDispatch *lpDisp;
			m_pdlEmrCollectionList->GetNextRowEnum(&pCurRowEnum, &lpDisp);
			pRow = lpDisp;
			lpDisp->Release();
			lpDisp = NULL;
		}

		ASSERT(pRow != NULL);
		_variant_t var = pRow->GetValue(eclcCollection);
		
		CString strCollectionName;
		strCollectionName = VarString(var);
		
		if(strCollectionNameToValidate.CompareNoCase(strCollectionName) == 0)
		{
			AfxMessageBox("Collection names must be unique");
			return false;
		}
	}
	return true;
}

void CEmrCollectionSetupDlg::OnEditingFinishingEmrcollectionList(long nRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	switch(nCol) {
	case eclcCollection:
		{
			CString strNewCollectionName;
			strNewCollectionName = VarString(pvarNewValue);
			strNewCollectionName.TrimLeft();
			strNewCollectionName.TrimRight();

			CString strOldCollectionName ;
			strOldCollectionName = VarString(varOldValue);
			strOldCollectionName.TrimLeft();
			strOldCollectionName.TrimRight();
			

			if(*pbCommit){
				if(IsCollectionNameValid(strNewCollectionName, strOldCollectionName)){
					*pbCommit = TRUE;
					*pbContinue = TRUE;
					return;
				}
				else{
					*pbCommit = FALSE;
					// (a.walling 2010-08-16 17:08) - PLID 40131 - Fix leak and crash
					VariantClear(pvarNewValue);
					*pvarNewValue = _variant_t(varOldValue).Detach();
					return;
				}
			}
		}
		break;
	}
}
