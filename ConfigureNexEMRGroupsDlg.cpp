// ConfigureNexEMRGroupsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ConfigureNexEMRGroupsDlg.h"
#include <afxtempl.h>

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CConfigureNexEMRGroupsDlg dialog

#define ID_REMOVE_GROUP_TEMPLATE	40000


CConfigureNexEMRGroupsDlg::CConfigureNexEMRGroupsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CConfigureNexEMRGroupsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CConfigureNexEMRGroupsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	m_nNewID = 0;
}


void CConfigureNexEMRGroupsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CConfigureNexEMRGroupsDlg)
	DDX_Control(pDX, IDC_SHOW_PREFERRED_PROC, m_checkShowPreferredProc);
	DDX_Control(pDX, IDC_MOVE_GROUP_TEMPLATE_PRIORITY_UP_BTN, m_btnGroupTemplatePriorityUp);
	DDX_Control(pDX, IDC_MOVE_GROUP_TEMPLATE_PRIORITY_DOWN_BTN, m_btnGroupTemplatePriorityDown);
	DDX_Control(pDX, IDC_MOVE_EMN_GROUP_PRIORITY_UP_BTN, m_btnMovePriorityUp);
	DDX_Control(pDX, IDC_MOVE_EMN_GROUP_PRIORITY_DOWN_BTN, m_btnMovePriorityDown);
	DDX_Control(pDX, IDC_DELETE_EMN_GROUP, m_btnDeleteEMNGroup);
	DDX_Control(pDX, IDC_ADD_EMN_GROUP, m_btnAddEMNGroup);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CConfigureNexEMRGroupsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CConfigureNexEMRGroupsDlg)
	ON_BN_CLICKED(IDC_ADD_EMN_GROUP, OnAddEmnGroup)
	ON_BN_CLICKED(IDC_DELETE_EMN_GROUP, OnDeleteEmnGroup)
	ON_BN_CLICKED(IDC_MOVE_EMN_GROUP_PRIORITY_DOWN_BTN, OnMoveEmnGroupPriorityDownBtn)
	ON_BN_CLICKED(IDC_MOVE_EMN_GROUP_PRIORITY_UP_BTN, OnMoveEmnGroupPriorityUpBtn)
	ON_COMMAND(ID_REMOVE_GROUP_TEMPLATE, OnRemoveGroupTemplate)
	ON_BN_CLICKED(IDC_MOVE_GROUP_TEMPLATE_PRIORITY_DOWN_BTN, OnMoveGroupTemplatePriorityDownBtn)
	ON_BN_CLICKED(IDC_MOVE_GROUP_TEMPLATE_PRIORITY_UP_BTN, OnMoveGroupTemplatePriorityUpBtn)
	ON_BN_CLICKED(IDC_SHOW_PREFERRED_PROC, OnShowPreferredProc)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CConfigureNexEMRGroupsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CConfigureNexEMRGroupsDlg)
	ON_EVENT(CConfigureNexEMRGroupsDlg, IDC_EMN_GROUPS_LIST, 2 /* SelChanged */, OnSelChangedEmnGroupsList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CConfigureNexEMRGroupsDlg, IDC_EMN_GROUPS_LIST, 9 /* EditingFinishing */, OnEditingFinishingEmnGroupsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CConfigureNexEMRGroupsDlg, IDC_EMN_GROUPS_LIST, 10 /* EditingFinished */, OnEditingFinishedEmnGroupsList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CConfigureNexEMRGroupsDlg, IDC_EMN_GROUPS_LIST, 18 /* RequeryFinished */, OnRequeryFinishedEmnGroupList, VTS_I2)
	ON_EVENT(CConfigureNexEMRGroupsDlg, IDC_EMN_GROUP_TEMPLT_COMBO, 16 /* SelChosen */, OnSelChosenEmnGroupTemplatesCombo, VTS_DISPATCH)
	ON_EVENT(CConfigureNexEMRGroupsDlg, IDC_EMN_GROUP_TEMPLT_LIST, 7 /* RButtonUp */, OnRButtonUpEmnGroupTemplateList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CConfigureNexEMRGroupsDlg, IDC_EMN_GROUP_TEMPLT_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedEmnGroupTempltCombo, VTS_I2)
	ON_EVENT(CConfigureNexEMRGroupsDlg, IDC_EMN_GROUP_TEMPLT_LIST, 2 /* SelChanged */, OnSelChangedEmnGroupTempltList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CConfigureNexEMRGroupsDlg, IDC_CONFIGURE_PREFERRED_PROC_LIST, 10 /* EditingFinished */, OnEditingFinishedConfigurePreferredProcList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfigureNexEMRGroupsDlg message handlers

BOOL CConfigureNexEMRGroupsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		// (z.manning 2009-08-11 16:36) - PLID 32989 - Added caching
		g_propManager.CachePropertiesInBulk("ConfigureNexEMRGroupsDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'ShowEMRPreferredProcedures' "
			//"Name = 'EMRAutoExpandGroupID' " // (z.manning 2009-08-11 16:49) - This gets cached in PatientNexEMRDlg
			")",
			_Q(GetCurrentUserName()));

		//ALTER TABLE ProcedureT ADD EMRPreferredProcedure BIT NOT NULL DEFAULT(0)
		m_pPreferredList = BindNxDataList2Ctrl(this, IDC_CONFIGURE_PREFERRED_PROC_LIST, GetRemoteData(), true);
		//DRT 1/16/2006 - PLID 18209 - After some discussion we threw out the subjective based, and combined it with the symptom based.
		//m_pSubjectiveList = BindNxDataList2Ctrl(this, IDC_CONFIGURE_SUBJECTIVE_GROUP, GetRemoteData(), true);

		// (a.wetta 2007-06-06 11:25) - PLID 26234 - The groups are now customizable
		m_pEMNGroupList = BindNxDataList2Ctrl(this, IDC_EMN_GROUPS_LIST, GetRemoteData(), true);
		m_pEMNGroupTemplateCombo = BindNxDataList2Ctrl(this, IDC_EMN_GROUP_TEMPLT_COMBO, GetRemoteData(), false);
		GetDlgItem(IDC_EMN_GROUP_TEMPLT_COMBO)->EnableWindow(FALSE);
		m_pEMNGroupTemplateList = BindNxDataList2Ctrl(this, IDC_EMN_GROUP_TEMPLT_LIST, GetRemoteData(), false);
		GetDlgItem(IDC_EMN_GROUP_TEMPLT_LIST)->EnableWindow(FALSE);

		m_btnMovePriorityUp.AutoSet(NXB_UP);
		m_btnMovePriorityDown.AutoSet(NXB_DOWN);
		m_btnGroupTemplatePriorityUp.AutoSet(NXB_UP);
		m_btnGroupTemplatePriorityDown.AutoSet(NXB_DOWN);
		// (c.haag 2008-05-01 10:51) - PLID 29863 - NxIconify additional buttons
		m_btnDeleteEMNGroup.AutoSet(NXB_DELETE);
		m_btnAddEMNGroup.AutoSet(NXB_NEW);
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		UpdateSortOrderButtons();
		UpdateTemplateSortOrderButtons();

		GetDlgItem(IDC_DELETE_EMN_GROUP)->EnableWindow(FALSE);

		if (GetRemotePropertyInt("ShowEMRPreferredProcedures", 1)) {
			CheckDlgButton(IDC_SHOW_PREFERRED_PROC, TRUE);
			GetDlgItem(IDC_CONFIGURE_PREFERRED_PROC_LIST)->EnableWindow(TRUE);
		}
		else {
			CheckDlgButton(IDC_SHOW_PREFERRED_PROC, FALSE);
			GetDlgItem(IDC_CONFIGURE_PREFERRED_PROC_LIST)->EnableWindow(FALSE);
		}

	} NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CConfigureNexEMRGroupsDlg::OnOK() 
{
	try {
		// (a.wetta 2007-06-06 14:55) - PLID 26234 - Save everything
		Save();

		CDialog::OnOK();
	} NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnOK");
}

void CConfigureNexEMRGroupsDlg::OnCancel() 
{
	try
	{
		// (a.wetta 2007-06-13 10:46) - PLID 26234 - Prompt them if they have made changes
		if (m_mapGroupChanges.GetCount() > 0 || m_mapGroupTemplateChanges.GetCount() > 0 || m_mapPreferredProcChanges.GetCount() > 0 ||
			GetRemotePropertyInt("ShowEMRPreferredProcedures", 1) != (long)IsDlgButtonChecked(IDC_SHOW_PREFERRED_PROC)) {
			
			if (MessageBox("Are you sure you wish to close without saving your changes?", NULL, MB_YESNO|MB_ICONQUESTION) != IDYES)
				return;
		}

		//Save nothing, the user has quit
		CDialog::OnCancel();

	}NxCatchAll("CConfigureNexEMRGroupsDlg::OnCancel");
}

//Given a v2 datalist, this function will see if column nCheckedColumn (of type VT_BOOL) is true, and if so, 
//	add the value of nIDColumn (of type VT_I4) to a comma delimited string.
// (a.wetta 2007-06-13 09:49) - PLID 26234 - The save functions that used this function are no longer used
/*CString GenerateIDListFromDatalist(NXDATALIST2Lib::_DNxDataListPtr pList, short nIDColumn, short nCheckedColumn)
{
	//array to hold our selected ids
	CDWordArray dwary;

	//Start looping at the first row
	NXDATALIST2Lib::IRowSettingsPtr pCurRow = pList->GetFirstRow();

	while(pCurRow) {
		//If it's checked, add to our list
		if(VarBool(pCurRow->GetValue(nCheckedColumn))) {
			dwary.Add(VarLong(pCurRow->GetValue(nIDColumn)));
		}

		//move to the next row
		pCurRow = pCurRow->GetNextRow();
	}

	CString strList, str;

	//now we traverse the entire list of procedures, and make a list of all
	//	procedure IDs which are checked.
	for(int i = 0; i < dwary.GetSize(); i++) {
		str.Format("%li, ", dwary.GetAt(i));
		strList += str;
	}

	//And now that we've finished with the loop, trim the extra ", " off the end
	strList.TrimRight(", ");

	return strList;
}*/

//TODO:  Currently, the datalist v2.0 is not yet completely converted, so I cannot use the EditingFinished 
//	event to watch for changes.  This code isn't too bad, but it might be better to come back and convert
//	the dialog to watch for changes and generate the same from its monitoring.
// (a.wetta 2007-06-13 09:49) - PLID 26234 - This is no longer used.  The changes are recorded as they are made and then handled in Save()
/*CString CConfigureNexEMRGroupsDlg::GeneratePreferredSaveQuery()
{
	//Get our list of selected IDs
	CString strIDList = GenerateIDListFromDatalist(m_pPreferredList, 0, 1);

	//Continue on and generate the query
	CString strQuery;
	CString str;

	//First we'll unselect everything
	str.Format("UPDATE ProcedureT SET EMRPreferredProcedure = 0;\r\n");
	strQuery += str;

	//if nothing was selected, we can quit on the spot, just wiping everything out to unselected
	if(strIDList.IsEmpty())
		return strQuery;

	//Next, we update only the IDs which are selected
	str.Format("UPDATE ProcedureT SET EMRPreferredProcedure = 1 WHERE ProcedureT.ID IN (");
	strQuery += str;

	//Add the list of IDs
	strQuery += strIDList;

	//Finish up the query syntax
	strQuery += ");\r\n";

	//And we've got ourselves a query
	return strQuery;
}*/

// (a.wetta 2007-06-06 14:52) - PLID 26234 - This is no longer used because the groups and their templates have been made customizable
/*CString CConfigureNexEMRGroupsDlg::GenerateSymptomSaveQuery()
{
	//Get our list of selected IDs
	CString strIDList = GenerateIDListFromDatalist(m_pSymptomList, 0, 1);

	//Continue on and generate the query
	CString strQuery;
	CString str;

	//First we'll unselect everything
	str.Format("UPDATE EMRTemplateT SET EMRSymptomGroup = 0;\r\n");
	strQuery += str;

	//if nothing was selected, we can quit on the spot, just wiping everything out to unselected
	if(strIDList.IsEmpty())
		return strQuery;

	//Next, we update only the IDs which are selected
	str.Format("UPDATE EMRTemplateT SET EMRSymptomGroup = 1 WHERE EMRTemplateT.ID IN (");
	strQuery += str;

	//Add the list of IDs
	strQuery += strIDList;

	//Finish up the query syntax
	strQuery += ");\r\n";

	//And we've got ourselves a query
	return strQuery;
}*/

/*	DRT 1/16/2006 - PLID 18209 - Removed subjective based
CString CConfigureNexEMRGroupsDlg::GenerateSubjectiveSaveQuery()
{
	//Get our list of selected IDs
	CString strIDList = GenerateIDListFromDatalist(m_pSubjectiveList, 0, 1);

	//Continue on and generate the query
	CString strQuery;
	CString str;

	//First we'll unselect everything
	str.Format("UPDATE EMRTemplateT SET EMRSubjectiveGroup = 0;\r\n");
	strQuery += str;

	//if nothing was selected, we can quit on the spot, just wiping everything out to unselected
	if(strIDList.IsEmpty())
		return strQuery;

	//Next, we update only the IDs which are selected
	str.Format("UPDATE EMRTemplateT SET EMRSubjectiveGroup = 1 WHERE EMRTemplateT.ID IN (");
	strQuery += str;

	//Add the list of IDs
	strQuery += strIDList;

	//Finish up the query syntax
	strQuery += ");\r\n";

	//And we've got ourselves a query
	return strQuery;
}
*/

void CConfigureNexEMRGroupsDlg::OnSelChangedEmnGroupsList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pOldSelRow(lpOldSel);
		NXDATALIST2Lib::IRowSettingsPtr pNewSelRow(lpNewSel);
		SelChangedEmnGroupsList(pOldSelRow, pNewSelRow);

	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnSelChangedEmnGroupsList");
}

// (a.wetta 2007-06-13 14:48) - PLID 26234 - Handle a group selection change
void CConfigureNexEMRGroupsDlg::SelChangedEmnGroupsList(NXDATALIST2Lib::IRowSettingsPtr pOldSel, NXDATALIST2Lib::IRowSettingsPtr pNewSel)
{
	try {
		if (pNewSel) {
			// Clear the template list
			m_pEMNGroupTemplateList->Clear();

			// Show the delete button
			GetDlgItem(IDC_DELETE_EMN_GROUP)->EnableWindow(TRUE);

			// Refresh the combo which will then also fill the template list when it's done requerying
			GetDlgItem(IDC_EMN_GROUP_TEMPLT_COMBO)->EnableWindow(FALSE);
			m_pEMNGroupTemplateCombo->Requery();

			GetDlgItem(IDC_EMN_GROUP_TEMPLT_LIST)->EnableWindow(TRUE);
		}
		else {
			m_pEMNGroupTemplateList->Clear();
			GetDlgItem(IDC_EMN_GROUP_TEMPLT_COMBO)->EnableWindow(FALSE);
			GetDlgItem(IDC_EMN_GROUP_TEMPLT_LIST)->EnableWindow(FALSE);

			// Hide the delete button
			GetDlgItem(IDC_DELETE_EMN_GROUP)->EnableWindow(FALSE);
		}

		UpdateSortOrderButtons();
		UpdateTemplateSortOrderButtons();

	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::SelChangedEmnGroupsList");
}

// (a.wetta 2007-06-13 14:48) - PLID 26234 - Add a new EMN template group
void CConfigureNexEMRGroupsDlg::OnAddEmnGroup() 
{
	try {
		// Get the currently displayed rule
		NXDATALIST2Lib::IRowSettingsPtr pOldSel = m_pEMNGroupList->GetCurSel();

		// Create the new row
		NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pEMNGroupList->GetNewRow();
		
		long nNewID = --m_nNewID;
		CString strNewName;
		strNewName.Format("<New Group %li>", -nNewID);
		// Find the next highest sort order number
		NXDATALIST2Lib::IRowSettingsPtr pSortRow = m_pEMNGroupList->GetFirstRow();
		long nNewSortOrder = -1;
		while (pSortRow) {
			if (VarLong(pSortRow->GetValue(egfSortOrder)) > nNewSortOrder)
				nNewSortOrder = VarLong(pSortRow->GetValue(egfSortOrder));
			pSortRow = pSortRow->GetNextRow();
		}
		nNewSortOrder++;

		// Add the new row to the datalist
		pNewRow->PutValue(egfID, _variant_t(nNewID));
		pNewRow->PutValue(egfName, _variant_t(strNewName));
		pNewRow->PutValue(egfSortOrder, _variant_t(nNewSortOrder));
		// (z.manning 2011-06-17 12:21) - PLID 37651 - Need to initialize the exapand column too
		pNewRow->PutValue(egfExpand, g_cvarFalse);
		m_pEMNGroupList->AddRowSorted(pNewRow, NULL);
		m_pEMNGroupList->SetSelByColumn(egfID,  _variant_t(nNewID));

		// Start Editing the name
		m_pEMNGroupList->StartEditing(pNewRow, egfName);

		// Handle the row change
		SelChangedEmnGroupsList(pOldSel, pNewRow);

		// Record the change
		EMNGroupChangeInfo egciChangeInfo;
		egciChangeInfo.nGroupID = nNewID;
		egciChangeInfo.strGroupName = strNewName;
		egciChangeInfo.strOriginalName = strNewName;
		egciChangeInfo.nSortOrder = nNewSortOrder;
		egciChangeInfo.nOriginalSortOrder = nNewSortOrder;
		egciChangeInfo.tegcTypeOfChange = tegcCreate;
		m_mapGroupChanges[nNewID] = egciChangeInfo;

		VerifyGroupSortOrders();
		UpdateSortOrderButtons();
		
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnAddEmnGroup");
}

// (a.wetta 2007-06-13 14:48) - PLID 26234 - Delete an EMN template group
void CConfigureNexEMRGroupsDlg::OnDeleteEmnGroup() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pEMNGroupList->GetCurSel();

		if (pRow) {
			if (IDNO == MessageBox(FormatString("Are you sure you want to delete the group '%s'?", VarString(pRow->GetValue(egfName))), NULL, MB_YESNO|MB_ICONQUESTION))
				return;

			// Delete the row from the datalist
			m_pEMNGroupList->RemoveRow(pRow);

			// Handle the row change
			SelChangedEmnGroupsList(pRow, m_pEMNGroupList->GetCurSel());

			// Update the changes lists
			long nGroupID = VarLong(pRow->GetValue(egfID));
			EMNGroupChangeInfo egciChangeInfo;
			if (m_mapGroupChanges.Lookup(nGroupID, egciChangeInfo)) {
				// Determine if this is an existing group or a new one
				if (egciChangeInfo.tegcTypeOfChange == tegcCreate) {
					// This is a new group, so just take it out of the list and it won't be saved
					m_mapGroupChanges.RemoveKey(nGroupID);
				}
				else {
					// This an existing group that is in the list because it had been modified, mark it as delete now
					egciChangeInfo.tegcTypeOfChange = tegcDelete;
					m_mapGroupChanges[nGroupID] = egciChangeInfo;
				}
			}
			else {
				// This is an existing group that isn't in the list because nothing has been done to it, add it to the list
				EMNGroupChangeInfo egciChangeInfo;
				egciChangeInfo.nGroupID = nGroupID;
				egciChangeInfo.strGroupName = VarString(pRow->GetValue(egfName));
				egciChangeInfo.strOriginalName = VarString(pRow->GetValue(egfName));
				egciChangeInfo.tegcTypeOfChange = tegcDelete;
				m_mapGroupChanges[nGroupID] = egciChangeInfo;
			}

			VerifyGroupSortOrders();
			UpdateSortOrderButtons();
		}
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnDeleteEmnGroup");	
}

// (a.wetta 2007-06-13 14:49) - PLID 26234 - Verify that the changes to the groups are valid
void CConfigureNexEMRGroupsDlg::OnEditingFinishingEmnGroupsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if (nCol == egfName && *pbCommit) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			CString strNewName = VarString(*pvarNewValue);

			// Make sure the name isn't blank
			if (strNewName.IsEmpty()) {
				MessageBox("The group name cannot be blank.  Please enter a group name.", NULL, MB_OK|MB_ICONWARNING);
				*pbContinue = FALSE;
				*pbCommit = FALSE;
			}

			// Make sure this name isn't already used
			NXDATALIST2Lib::IRowSettingsPtr pCompareRow = m_pEMNGroupList->GetFirstRow();
			while (pCompareRow) {
				if (pCompareRow != pRow && VarString(pCompareRow->GetValue(egfName)) == strNewName) {
					// This name already exists for a differ group
					MessageBox("This group name is already in use.  Please enter a different group name.", NULL, MB_OK|MB_ICONWARNING);
					*pbContinue = FALSE;
					*pbCommit = FALSE;
				}
				pCompareRow = pCompareRow->GetNextRow();
			}
		}		
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnEditingFinishingEmnGroupsList");
}

// (a.wetta 2007-06-13 14:50) - PLID 26234 - Be sure to record any changes to the groups
void CConfigureNexEMRGroupsDlg::OnEditingFinishedEmnGroupsList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if (lpRow && bCommit) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			RecordEMNGroupChange((EMNGroupFields)nCol, pRow, varOldValue, varNewValue);
		}
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnEditingFinishedEmnGroupsList");
}

// (a.wetta 2007-06-13 14:51) - PLID 26234 - This function will record a change made to a group so that it can be undated in the data when
// everything is saved
void CConfigureNexEMRGroupsDlg::RecordEMNGroupChange(EMNGroupFields egfChangeField, NXDATALIST2Lib::IRowSettingsPtr pChangeRow, _variant_t varOldValue, _variant_t varNewValue)
{
	try {
		if (egfChangeField == egfName) {
			if (VarString(varOldValue) == VarString(varNewValue))
				// Nothing was actually changed
				return;

			// Let's add this change to our list
			long nGroupID = VarLong(pChangeRow->GetValue(egfID));
			EMNGroupChangeInfo egciChangeInfo;
			if (m_mapGroupChanges.Lookup(nGroupID, egciChangeInfo)) {
				// The group is already in the list
				if (egciChangeInfo.tegcTypeOfChange == tegcDelete) {
					// The group seems to be deleted but somehow we have modified it; this should not be possible
					ASSERT(FALSE);
				}
				else if (egciChangeInfo.tegcTypeOfChange == tegcModify) {
					if (egciChangeInfo.strOriginalName == VarString(varNewValue) && 
						egciChangeInfo.nSortOrder == egciChangeInfo.nOriginalSortOrder) {
						// They have just changed the group back to its original values, there is no need for this change entry anymore
						m_mapGroupChanges.RemoveKey(nGroupID);
					}
					else {
						egciChangeInfo.strGroupName = VarString(varNewValue);
						m_mapGroupChanges[nGroupID] = egciChangeInfo;
					}
				}
				else if (egciChangeInfo.tegcTypeOfChange == tegcCreate) {
					// Just update the name
					egciChangeInfo.strGroupName = VarString(varNewValue);
					m_mapGroupChanges[nGroupID] = egciChangeInfo;
				}
			}
			else {
				// This is an existing group that isn't in the list because nothing has been done to it, add it to the list
				EMNGroupChangeInfo egciChangeInfo;
				egciChangeInfo.nGroupID = nGroupID;
				egciChangeInfo.strGroupName = VarString(varNewValue);
				egciChangeInfo.strOriginalName = VarString(varOldValue);
				egciChangeInfo.nSortOrder = VarLong(pChangeRow->GetValue(egfSortOrder));
				egciChangeInfo.nOriginalSortOrder = VarLong(pChangeRow->GetValue(egfSortOrder));
				egciChangeInfo.tegcTypeOfChange = tegcModify;
				m_mapGroupChanges[nGroupID] = egciChangeInfo;
			}
		}
		else if (egfChangeField == egfSortOrder) {
			if (VarLong(varOldValue) == VarLong(varNewValue))
				// Nothing was actually changed
				return;

			// Let's add this change to our list
			long nGroupID = VarLong(pChangeRow->GetValue(egfID));
			EMNGroupChangeInfo egciChangeInfo;
			if (m_mapGroupChanges.Lookup(nGroupID, egciChangeInfo)) {
				// The group is already in the list
				if (egciChangeInfo.tegcTypeOfChange == tegcDelete) {
					// The group seems to be deleted but somehow we have modified it; this should not be possible
					ASSERT(FALSE);
				}
				else if (egciChangeInfo.tegcTypeOfChange == tegcModify) {
					if (egciChangeInfo.nOriginalSortOrder == VarLong(varNewValue) && 
						egciChangeInfo.strGroupName == egciChangeInfo.strOriginalName) {
						// They have just changed the group back to its original values, there is no need for this change entry anymore
						m_mapGroupChanges.RemoveKey(nGroupID);
					}
					else {
						egciChangeInfo.nSortOrder = VarLong(varNewValue);
						m_mapGroupChanges[nGroupID] = egciChangeInfo;
					}
				}
				else if (egciChangeInfo.tegcTypeOfChange == tegcCreate) {
					// Just update the sort order
					egciChangeInfo.nSortOrder = VarLong(varNewValue);
					m_mapGroupChanges[nGroupID] = egciChangeInfo;
				}
			}
			else {
				// This is an existing group that isn't in the list because nothing has been done to it, add it to the list
				EMNGroupChangeInfo egciChangeInfo;
				egciChangeInfo.nGroupID = nGroupID;
				egciChangeInfo.nSortOrder = VarLong(varNewValue);
				egciChangeInfo.nOriginalSortOrder = VarLong(varOldValue);
				egciChangeInfo.strGroupName = VarString(pChangeRow->GetValue(egfName));
				egciChangeInfo.strOriginalName = VarString(pChangeRow->GetValue(egfName));
				egciChangeInfo.tegcTypeOfChange = tegcModify;
				m_mapGroupChanges[nGroupID] = egciChangeInfo;
			}
		}
		else if(egfChangeField == egfExpand)
		{
			// (z.manning 2009-08-11 16:19) - PLID 32989 - To be consistent with pre-existing 
			// behavior in the new EMR list, let's only let them choose at most one group to expand.
			BOOL bExpand = VarBool(varNewValue);
			if(bExpand) {
				NXDATALIST2Lib::IRowSettingsPtr pTemp;
				for(pTemp = m_pEMNGroupList->GetFirstRow(); pTemp != NULL; pTemp = pTemp->GetNextRow()) {
					if(pTemp != pChangeRow) {
						pTemp->PutValue(egfExpand, g_cvarFalse);
					}
				}
			}
		}

	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::RecordEMNGroupChange");
}

// (a.wetta 2007-06-13 14:51) - PLID 26234 - Handle adding a new template to a group
void CConfigureNexEMRGroupsDlg::OnSelChosenEmnGroupTemplatesCombo(LPDISPATCH lpRow) 
{
	try {
		if (lpRow) {
			NXDATALIST2Lib::IRowSettingsPtr pComboRow(lpRow);

			// Create the new row for the template list.
			NXDATALIST2Lib::IRowSettingsPtr pNewRow = m_pEMNGroupTemplateList->GetNewRow();
			pNewRow->PutValue(egtfID, pComboRow->GetValue(egtfID));
			pNewRow->PutValue(egtfName, pComboRow->GetValue(egtfName));
			pNewRow->PutValue(egtfCollection, pComboRow->GetValue(egtfCollection));
			
			// Find the next highest sort order number
			NXDATALIST2Lib::IRowSettingsPtr pSortRow = m_pEMNGroupTemplateList->GetFirstRow();
			long nNewSortOrder = -1;
			while (pSortRow) {
				if (VarLong(pSortRow->GetValue(egtfSortOrder)) > nNewSortOrder)
					nNewSortOrder = VarLong(pSortRow->GetValue(egtfSortOrder));
				pSortRow = pSortRow->GetNextRow();
			}
			nNewSortOrder++;
			// Update the sort order of the new row
			pNewRow->PutValue(egtfSortOrder, nNewSortOrder);
			// Add the row to the template list and remove it from the template combo so it can't be added more than once.
			m_pEMNGroupTemplateList->AddRowAtEnd(pNewRow, NULL);
			m_pEMNGroupTemplateCombo->RemoveRow(pComboRow);
			GetDlgItem(IDC_EMN_GROUP_TEMPLT_LIST)->SetFocus();
			m_pEMNGroupTemplateList->PutCurSel(pNewRow);

			// Let's add this change to our list
			NXDATALIST2Lib::IRowSettingsPtr pGroupRow = m_pEMNGroupList->GetCurSel();
			long nGroupID = VarLong(pGroupRow->GetValue(egfID));
			long nTemplateID = VarLong(pNewRow->GetValue(egtfID));
			CString strMapKey;
			strMapKey.Format("%li-%li", nGroupID, nTemplateID);
			EMNGroupTemplateChangeInfo egtciChangeInfo;
			if (m_mapGroupTemplateChanges.Lookup(strMapKey, egtciChangeInfo)) {
				// If a change entry already exists, then that means that the template was removed and then readded to the group before saving,
				// so effectively no change in the data
				if (egtciChangeInfo.nOriginalSortOrder == nNewSortOrder) {
					m_mapGroupTemplateChanges.RemoveKey(strMapKey);
				}
				else {
					// But if the sort order has changed, then this template has been modified
					egtciChangeInfo.nSortOrder = nNewSortOrder;
					egtciChangeInfo.teglcTypeOfChange = teglcModify;
					m_mapGroupTemplateChanges[strMapKey] = egtciChangeInfo;
				}
			}
			else {
				// Add the change into to the list
				egtciChangeInfo.nGroupID = nGroupID;
				egtciChangeInfo.nTemplateID = nTemplateID;
				egtciChangeInfo.nSortOrder = nNewSortOrder;
				egtciChangeInfo.nOriginalSortOrder = nNewSortOrder;
				egtciChangeInfo.teglcTypeOfChange = teglcCreateLink;
				m_mapGroupTemplateChanges[strMapKey] = egtciChangeInfo;
			}

			VerifyTemplateSortOrders();
			UpdateTemplateSortOrderButtons();
		}
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnSelChosenEmnGroupTemplatesCombo");	
}

// (a.wetta 2007-06-11 16:04) - PLID 26234 - Save all of the changes made on the dialog
void CConfigureNexEMRGroupsDlg::Save()
{
	try {
		// Start the sql batch to update the data
		bool bFoundChanges = false;

		// (z.manning 2011-06-22 17:12) - PLID 37651 - Changed all the saving here to now use a CParamSqlBatch
		CParamSqlBatch sqlBatch;
		sqlBatch.Add("SET NOCOUNT ON");
		sqlBatch.Declare("DECLARE @NewGroupMap TABLE (ID int NOT NULL, MapID int NOT NULL)");

		// Update all of the group info
		long nMapID;
		EMNGroupChangeInfo egciChangeInfo;
		POSITION posMap = m_mapGroupChanges.GetStartPosition();
		while (posMap)
		{
			m_mapGroupChanges.GetNextAssoc(posMap, nMapID, egciChangeInfo);
			CString strGroupName = egciChangeInfo.strGroupName;

			if (egciChangeInfo.tegcTypeOfChange == tegcCreate) {
				// (z.manning 2011-06-22 17:42) - PLID 37651 - Keep track of where in the map that each new group is located.
				sqlBatch.Add(
					"INSERT INTO EMNTemplateGroupsT (Name, SortOrder) VALUES ({STRING}, {INT}) \r\n"
					"INSERT INTO @NewGroupMap (ID, MapID) SELECT SCOPE_IDENTITY(), {INT} "
					, strGroupName, egciChangeInfo.nSortOrder, (long)nMapID);
			}
			else if (egciChangeInfo.tegcTypeOfChange == tegcModify) {
				sqlBatch.Add("UPDATE EMNTemplateGroupsT SET Name = {STRING}, SortOrder = {INT} WHERE ID = {INT}", strGroupName, egciChangeInfo.nSortOrder, egciChangeInfo.nGroupID);
			}
			else if (egciChangeInfo.tegcTypeOfChange == tegcDelete) {
				sqlBatch.Add("DELETE FROM EMNTemplateGroupsLinkT WHERE EMNTemplateGroupID = {INT}", egciChangeInfo.nGroupID);
				sqlBatch.Add("DELETE FROM EMNTemplateGroupsT WHERE ID = {INT}", egciChangeInfo.nGroupID);
			}

			bFoundChanges = true;
		}

		// Update all fo the templates associated with a group
		sqlBatch.Declare("DECLARE @nNewEMNTemplateGroupID int");
		CString strMapID;
		EMNGroupTemplateChangeInfo egtciTemplateChangeInfo;
		posMap = m_mapGroupTemplateChanges.GetStartPosition();
		while (posMap) {
			m_mapGroupTemplateChanges.GetNextAssoc(posMap, strMapID, egtciTemplateChangeInfo);

			// Make sure that the group still exists and hasn't been deleted
			NXDATALIST2Lib::IRowSettingsPtr pGroupRow = m_pEMNGroupList->FindByColumn(egfID, _variant_t(egtciTemplateChangeInfo.nGroupID), NULL, FALSE);
			if (pGroupRow) {
				if (egtciTemplateChangeInfo.nGroupID < 0) {
					// This is a new group, get it's new ID
					if (m_mapGroupChanges.Lookup(egtciTemplateChangeInfo.nGroupID, egciChangeInfo)) {
						CString strGroupName = egciChangeInfo.strGroupName;
						sqlBatch.Add("SET @nNewEMNTemplateGroupID = (SELECT MAX(ID) FROM EMNTemplateGroupsT WHERE Name = {STRING})", strGroupName);
						if (egtciTemplateChangeInfo.teglcTypeOfChange == teglcCreateLink) {
							sqlBatch.Add("INSERT INTO EMNTemplateGroupsLinkT (EMNTemplateGroupID, EMRTemplateID, SortOrder) VALUES (@nNewEMNTemplateGroupID, {INT}, {INT})", 
									egtciTemplateChangeInfo.nTemplateID, egtciTemplateChangeInfo.nSortOrder);
						}
						else if (egtciTemplateChangeInfo.teglcTypeOfChange == teglcModify) {
							sqlBatch.Add("UPDATE EMNTemplateGroupsLinkT SET SortOrder = {INT} WHERE EMNTemplateGroupID = @nNewEMNTemplateGroupID AND EMRTemplateID = {INT}", 
									egtciTemplateChangeInfo.nSortOrder, egtciTemplateChangeInfo.nTemplateID);
						}
						else if (egtciTemplateChangeInfo.teglcTypeOfChange == teglcDeleteLink) {
							sqlBatch.Add("DELETE FROM EMNTemplateGroupsLinkT WHERE EMNTemplateGroupID = @nNewEMNTemplateGroupID AND EMRTemplateID = {INT}",
									egtciTemplateChangeInfo.nTemplateID);
						}
					}
					else {
						// There shouldn't be a reason why this new group's ID isn't in the change map
						ASSERT(FALSE);
					}
				}
				else {
					// This is an existing group change
					if (egtciTemplateChangeInfo.teglcTypeOfChange == teglcCreateLink) {
						sqlBatch.Add("INSERT INTO EMNTemplateGroupsLinkT (EMNTemplateGroupID, EMRTemplateID, SortOrder) VALUES ({INT}, {INT}, {INT})", 
								egtciTemplateChangeInfo.nGroupID, egtciTemplateChangeInfo.nTemplateID, egtciTemplateChangeInfo.nSortOrder);
					}
					else if (egtciTemplateChangeInfo.teglcTypeOfChange == teglcModify) {
						sqlBatch.Add("UPDATE EMNTemplateGroupsLinkT SET SortOrder = {INT} WHERE EMNTemplateGroupID = {INT} AND EMRTemplateID = {INT}", 
								egtciTemplateChangeInfo.nSortOrder, egtciTemplateChangeInfo.nGroupID, egtciTemplateChangeInfo.nTemplateID);
					}
					else if (egtciTemplateChangeInfo.teglcTypeOfChange == teglcDeleteLink) {
						sqlBatch.Add("DELETE FROM EMNTemplateGroupsLinkT WHERE EMNTemplateGroupID = {INT} AND EMRTemplateID = {INT}",
								egtciTemplateChangeInfo.nGroupID, egtciTemplateChangeInfo.nTemplateID);
					}
				}
			}

			bFoundChanges = true;
		}

		// Update all of the preferred procedure info
		PreferredProcChangeInfo ppciChangeInfo;
		posMap = m_mapPreferredProcChanges.GetStartPosition();
		while (posMap) {
			m_mapPreferredProcChanges.GetNextAssoc(posMap, nMapID, ppciChangeInfo);

			sqlBatch.Add("UPDATE ProcedureT SET EMRPreferredProcedure = {INT} WHERE ID = {INT}", ppciChangeInfo.bChecked, ppciChangeInfo.nProcedureID);

			bFoundChanges = true;
		} 

		if (bFoundChanges) {
			sqlBatch.Add("SET NOCOUNT OFF");
			sqlBatch.Add("SELECT ID, MapID FROM @NewGroupMap");

			// (z.manning 2011-06-23 12:50) - PLID 37651 - Now go through any new groups and update each's corresponding row
			// with the new ID value.
			_RecordsetPtr prsSave = sqlBatch.CreateRecordset(GetRemoteData());
			for(; !prsSave->eof; prsSave->MoveNext()) {
				long nMapID = AdoFldLong(prsSave, "MapID");
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pEMNGroupList->FindByColumn(egfID, nMapID, NULL, VARIANT_FALSE);
				if(pRow != NULL) {
					long nGroupID = AdoFldLong(prsSave, "ID");
					pRow->PutValue(egfID, nGroupID);
				}
			}

			// (a.walling 2010-08-31 12:34) - PLID 29140 - Prevent refreshing the new list more often than necessary
			// Send an EMRTemplateT table checker so everyone refreshes the template group list
			// (j.jones 2014-08-12 10:17) - PLID 63189 - we now call RefreshEMRTemplateTable
			// to send an EMRTemplateT tablechecker, which also tells some of our local
			// lists to refresh
			RefreshEMRTemplateTable();
		}

		// Save whether the preferred procedures should be shown
		SetRemotePropertyInt("ShowEMRPreferredProcedures", IsDlgButtonChecked(IDC_SHOW_PREFERRED_PROC));

		// (z.manning 2009-08-11 16:34) - PLID 32989 - If they checked a group to auto-expand in the
		// new EMR list, save that.
		NXDATALIST2Lib::IRowSettingsPtr pGroupExpandRow = m_pEMNGroupList->FindByColumn(egfExpand, g_cvarTrue, NULL, VARIANT_FALSE);
		long nExpandGroupID = -1;
		if(pGroupExpandRow != NULL) {
			nExpandGroupID = VarLong(pGroupExpandRow->GetValue(egfID));
		}
		SetRemotePropertyInt("EMRAutoExpandGroupID", nExpandGroupID, 0, GetCurrentUserName());

		m_mapGroupChanges.RemoveAll();
		m_mapGroupTemplateChanges.RemoveAll();
		m_mapPreferredProcChanges.RemoveAll();

	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::Save");
}

void CConfigureNexEMRGroupsDlg::OnMoveEmnGroupPriorityDownBtn() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow1 = m_pEMNGroupList->GetCurSel();
		NXDATALIST2Lib::IRowSettingsPtr pRow2 = pRow1->GetNextRow();
		
		SwapEMNGroupSortOrders(pRow1, pRow2);

		// (z.manning, 09/12/2007) - PLID 26234 - Ensure the row is visible.
		m_pEMNGroupList->EnsureRowInView(pRow1);

		UpdateSortOrderButtons();
	
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnMoveEmnGroupPriorityDownBtn");
}

void CConfigureNexEMRGroupsDlg::OnMoveEmnGroupPriorityUpBtn() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow1 = m_pEMNGroupList->GetCurSel();
		NXDATALIST2Lib::IRowSettingsPtr pRow2 = pRow1->GetPreviousRow();
		
		SwapEMNGroupSortOrders(pRow1, pRow2);

		// (z.manning, 09/12/2007) - PLID 26234 - Ensure the row is visible.
		m_pEMNGroupList->EnsureRowInView(pRow1);

		UpdateSortOrderButtons();
	
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnMoveEmnGroupPriorityUpBtn");
}

// (a.wetta 2007-06-13 14:52) - PLID 26234 - Swap the sort order of the two group rows
void CConfigureNexEMRGroupsDlg::SwapEMNGroupSortOrders(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2)
{
	try {
		// Get the sort order values
		long nSortOrder1 = VarLong(pRow1->GetValue(egfSortOrder));
		long nSortOrder2 = VarLong(pRow2->GetValue(egfSortOrder));

		// Swap the values
		pRow1->PutValue(egfSortOrder, _variant_t(nSortOrder2));
		pRow2->PutValue(egfSortOrder, _variant_t(nSortOrder1));

		// Record the change
		RecordEMNGroupChange(egfSortOrder, pRow1, _variant_t(nSortOrder1), _variant_t(nSortOrder2));
		RecordEMNGroupChange(egfSortOrder, pRow2, _variant_t(nSortOrder2), _variant_t(nSortOrder1));
		
		// Sort the list
		m_pEMNGroupList->Sort();

		// Make sure row 1 is still selected
		m_pEMNGroupList->SetSelByColumn(egfID, pRow1->GetValue(egfID));

		VerifyGroupSortOrders();

	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::SwapEMNGroupSortOrders");
}

// (a.wetta 2007-06-13 14:52) - PLID 26234 - Make sure that the group sort order buttons are enabled properly
void CConfigureNexEMRGroupsDlg::UpdateSortOrderButtons()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pEMNGroupList->GetCurSel();
		if (pCurRow == NULL) {
			GetDlgItem(IDC_MOVE_EMN_GROUP_PRIORITY_UP_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_MOVE_EMN_GROUP_PRIORITY_DOWN_BTN)->EnableWindow(FALSE);
		}
		else {
			if (pCurRow->GetPreviousRow() == NULL) {
				GetDlgItem(IDC_MOVE_EMN_GROUP_PRIORITY_UP_BTN)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_MOVE_EMN_GROUP_PRIORITY_UP_BTN)->EnableWindow(TRUE);
			}

			if (pCurRow->GetNextRow() == NULL) {
				GetDlgItem(IDC_MOVE_EMN_GROUP_PRIORITY_DOWN_BTN)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_MOVE_EMN_GROUP_PRIORITY_DOWN_BTN)->EnableWindow(TRUE);
			}
		}
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::UpdateSortOrderButtons");
}

// (a.wetta 2007-06-13 14:53) - PLID 26234 - Create a pop up menu in the group template list when the user right clicks
void CConfigureNexEMRGroupsDlg::OnRButtonUpEmnGroupTemplateList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		if(lpRow) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			m_pEMNGroupTemplateList->SetSelByColumn(egtfID, pRow->GetValue(egtfID));

			CMenu mPopup;
			mPopup.CreatePopupMenu();
			mPopup.InsertMenu(0, MF_BYPOSITION, ID_REMOVE_GROUP_TEMPLATE, "Remove");

			CPoint pt;
			GetCursorPos(&pt);
			mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		}
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnRButtonUpEmnGroupTemplateList");
}

// (a.wetta 2007-06-13 14:53) - PLID 26234 - Handle removing a template from a group
void CConfigureNexEMRGroupsDlg::OnRemoveGroupTemplate()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pEMNGroupTemplateList->GetCurSel();

		if (pRow) {
			// Put the row back in the available templates combo
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
			m_pEMNGroupTemplateCombo->TakeRowAddSorted(pRow);
			m_pEMNGroupTemplateCombo->Sort();

			// Let's add this change to our list
			NXDATALIST2Lib::IRowSettingsPtr pGroupRow = m_pEMNGroupList->GetCurSel();
			long nGroupID = VarLong(pGroupRow->GetValue(egfID));
			long nTemplateID = VarLong(pRow->GetValue(egtfID));
			CString strMapKey;
			strMapKey.Format("%li-%li", nGroupID, nTemplateID);
			EMNGroupTemplateChangeInfo egtciChangeInfo;
			if (m_mapGroupTemplateChanges.Lookup(strMapKey, egtciChangeInfo)) {
				// If a create entry already exists, then that means that the template was added and then removed again from the group before saving,
				// so effectively no change in the data
				if (egtciChangeInfo.teglcTypeOfChange == teglcCreateLink) {
						m_mapGroupTemplateChanges.RemoveKey(strMapKey);
				}
				else {
					// This entry link exists in the data, so it needs deleted
					egtciChangeInfo.teglcTypeOfChange = teglcDeleteLink;
					m_mapGroupTemplateChanges[strMapKey] = egtciChangeInfo;
				}
			}
			else {
				// Add the change into to the list
				egtciChangeInfo.nGroupID = nGroupID;
				egtciChangeInfo.nTemplateID = nTemplateID;
				egtciChangeInfo.nSortOrder = VarLong(pRow->GetValue(egtfSortOrder));
				egtciChangeInfo.nOriginalSortOrder = VarLong(pRow->GetValue(egtfSortOrder));
				egtciChangeInfo.teglcTypeOfChange = teglcDeleteLink;
				m_mapGroupTemplateChanges[strMapKey] = egtciChangeInfo;
			}	

			VerifyTemplateSortOrders();
			UpdateTemplateSortOrderButtons();
		}
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnRemoveGroupTemplate");

}

// (a.wetta 2007-06-13 14:54) - PLID 26234 - When the templates combo is done requerying, all of the templates in the currently selected group
// have to be pulled out of the combo and put in the template list
void CConfigureNexEMRGroupsDlg::OnRequeryFinishedEmnGroupTempltCombo(short nFlags) 
{
	try {
		if (nFlags == NXDATALIST2Lib::dlRequeryFinishedCompleted) {
			// Get the currently selected group row
			NXDATALIST2Lib::IRowSettingsPtr pGroupRow = m_pEMNGroupList->GetCurSel();
			long nGroupID = -1;
			if (pGroupRow) {
				// Get the group ID
				nGroupID = VarLong(pGroupRow->GetValue(egfID));
			}

			// Add all of the templates to the list that need added to the template list
			if (nGroupID != -1) {
				_RecordsetPtr prs = CreateRecordset("SELECT EMRTemplateID, EMNTemplateGroupID, SortOrder FROM EMNTemplateGroupsLinkT "
													"WHERE EMNTemplateGroupID = %li "
													"UNION "
													"SELECT ID AS EMRTemplateID, -1 AS EMNTemplateGroupID, -1 AS SortOrder FROM EMRTemplateT "
													"WHERE ID NOT IN (SELECT EMRTemplateID FROM EMNTemplateGroupsLinkT WHERE EMNTemplateGroupID = %li)",
													nGroupID, nGroupID);
				while(!prs->eof) {
					long nTemplateID = AdoFldLong(prs, "EMRTemplateID");
					NXDATALIST2Lib::IRowSettingsPtr pTemplateRow = m_pEMNGroupTemplateCombo->FindByColumn(egtfID, nTemplateID, NULL, VARIANT_FALSE);
					// Make sure the user hasn't delete the template from the list and it just hasn't been saved in the data yet
					CString strMapKey;
					strMapKey.Format("%li-%li", nGroupID, nTemplateID);
					EMNGroupTemplateChangeInfo egtciChangeInfo;
					if(pTemplateRow) {
						if (m_mapGroupTemplateChanges.Lookup(strMapKey, egtciChangeInfo)) {
							if (egtciChangeInfo.teglcTypeOfChange != teglcDeleteLink) {
								pTemplateRow->PutValue(egtfSortOrder, _variant_t(egtciChangeInfo.nSortOrder));
								// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
								m_pEMNGroupTemplateList->TakeRowAddSorted(pTemplateRow);
							}
						}
						else {
							if (AdoFldLong(prs, "EMNTemplateGroupID") == nGroupID) {
								pTemplateRow->PutValue(egtfSortOrder, _variant_t(AdoFldLong(prs, "SortOrder")));
								// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method
								m_pEMNGroupTemplateList->TakeRowAddSorted(pTemplateRow);
							}
						}
					}
					prs->MoveNext();
				}
			}

			GetDlgItem(IDC_EMN_GROUP_TEMPLT_COMBO)->EnableWindow(TRUE);
		}
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnRequeryFinishedEmnGroupTempltCombo");	
}

void CConfigureNexEMRGroupsDlg::OnMoveGroupTemplatePriorityDownBtn() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow1 = m_pEMNGroupTemplateList->GetCurSel();
		NXDATALIST2Lib::IRowSettingsPtr pRow2 = pRow1->GetNextRow();
		
		SwapEMNGroupTemplateSortOrders(pRow1, pRow2);

		// (z.manning, 09/12/2007) - PLID 26234 - Ensure the row we're moving is visible.
		m_pEMNGroupTemplateList->EnsureRowInView(pRow1);

		UpdateTemplateSortOrderButtons();
	
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnMoveGroupTemplatePriorityDownBtn");
}

void CConfigureNexEMRGroupsDlg::OnMoveGroupTemplatePriorityUpBtn() 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow1 = m_pEMNGroupTemplateList->GetCurSel();
		NXDATALIST2Lib::IRowSettingsPtr pRow2 = pRow1->GetPreviousRow();
		
		SwapEMNGroupTemplateSortOrders(pRow1, pRow2);

		// (z.manning, 09/12/2007) - PLID 26234 - Ensure the row we're moving is visible.
		m_pEMNGroupTemplateList->EnsureRowInView(pRow1);

		UpdateTemplateSortOrderButtons();
	
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnMoveEmnGroupPriorityUpBtn");
}

// (a.wetta 2007-06-13 14:55) - PLID 26234 - Make sure that the template sort order buttons are enabled properly
void CConfigureNexEMRGroupsDlg::UpdateTemplateSortOrderButtons()
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pCurRow = m_pEMNGroupTemplateList->GetCurSel();
		if (pCurRow == NULL) {
			GetDlgItem(IDC_MOVE_GROUP_TEMPLATE_PRIORITY_UP_BTN)->EnableWindow(FALSE);
			GetDlgItem(IDC_MOVE_GROUP_TEMPLATE_PRIORITY_DOWN_BTN)->EnableWindow(FALSE);
		}
		else {
			if (pCurRow->GetPreviousRow() == NULL) {
				GetDlgItem(IDC_MOVE_GROUP_TEMPLATE_PRIORITY_UP_BTN)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_MOVE_GROUP_TEMPLATE_PRIORITY_UP_BTN)->EnableWindow(TRUE);
			}

			if (pCurRow->GetNextRow() == NULL) {
				GetDlgItem(IDC_MOVE_GROUP_TEMPLATE_PRIORITY_DOWN_BTN)->EnableWindow(FALSE);
			}
			else {
				GetDlgItem(IDC_MOVE_GROUP_TEMPLATE_PRIORITY_DOWN_BTN)->EnableWindow(TRUE);
			}
		}
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::UpdateTemplateSortOrderButtons");
}

// (a.wetta 2007-06-13 14:57) - PLID 26234 - Handle swapping the sort order of two template rows
void CConfigureNexEMRGroupsDlg::SwapEMNGroupTemplateSortOrders(NXDATALIST2Lib::IRowSettingsPtr pRow1, NXDATALIST2Lib::IRowSettingsPtr pRow2)
{
	try {
		// Get the sort order values
		long nSortOrder1 = VarLong(pRow1->GetValue(egtfSortOrder));
		long nSortOrder2 = VarLong(pRow2->GetValue(egtfSortOrder));

		// Swap the values
		pRow1->PutValue(egtfSortOrder, _variant_t(nSortOrder2));
		pRow2->PutValue(egtfSortOrder, _variant_t(nSortOrder1));

		// Record the change
		RecordEMNGroupTemplateChange(egtfSortOrder, pRow1, _variant_t(nSortOrder1), _variant_t(nSortOrder2));
		RecordEMNGroupTemplateChange(egtfSortOrder, pRow2, _variant_t(nSortOrder2), _variant_t(nSortOrder1));
		
		// Sort the list
		m_pEMNGroupTemplateList->Sort();

		// Make sure row 1 is still selected
		m_pEMNGroupTemplateList->SetSelByColumn(egtfID, pRow1->GetValue(egtfID));

		VerifyTemplateSortOrders();

	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::SwapEMNGroupTemplateSortOrders");
}

// (a.wetta 2007-06-13 14:58) - PLID 26234 - Record the specified change to a template associated with a group
void CConfigureNexEMRGroupsDlg::RecordEMNGroupTemplateChange(EMNGroupTemplateFields egtfChangeField, NXDATALIST2Lib::IRowSettingsPtr pChangeRow, _variant_t varOldValue, _variant_t varNewValue)
{
	try {
		if (egtfChangeField == egtfSortOrder) {
			if (VarLong(varOldValue) == VarLong(varNewValue))
				// Nothing was actually changed
				return;

			// Let's add this change to our list
			NXDATALIST2Lib::IRowSettingsPtr pGroupRow = m_pEMNGroupList->GetCurSel();
			long nGroupID = VarLong(pGroupRow->GetValue(egfID));
			long nTemplateID = VarLong(pChangeRow->GetValue(egtfID));
			CString strMapKey;
			strMapKey.Format("%li-%li", nGroupID, nTemplateID);
			EMNGroupTemplateChangeInfo egtciChangeInfo;
			if (m_mapGroupTemplateChanges.Lookup(strMapKey, egtciChangeInfo)) {
				// A change entry already exists
				if (egtciChangeInfo.nOriginalSortOrder == VarLong(varNewValue) && egtciChangeInfo.teglcTypeOfChange == teglcModify) {
					// They changed the sort order back, no change to the data necessary
					m_mapGroupTemplateChanges.RemoveKey(strMapKey);
				}
				else {
					// Update the sort order for the change entry
					egtciChangeInfo.nSortOrder = VarLong(varNewValue);
					m_mapGroupTemplateChanges[strMapKey] = egtciChangeInfo;
				}
			}
			else {
				// Add the change into to the list
				egtciChangeInfo.nGroupID = nGroupID;
				egtciChangeInfo.nTemplateID = nTemplateID;
				egtciChangeInfo.nSortOrder = VarLong(varNewValue);
				egtciChangeInfo.nOriginalSortOrder = VarLong(varOldValue);
				egtciChangeInfo.teglcTypeOfChange = teglcModify;
				m_mapGroupTemplateChanges[strMapKey] = egtciChangeInfo;
			}
		}
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::RecordEMNGroupTemplateChange");
}

void CConfigureNexEMRGroupsDlg::OnSelChangedEmnGroupTempltList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		UpdateTemplateSortOrderButtons();

	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnSelChangedEmnGroupTempltList");
}

// (a.wetta 2007-06-13 14:58) - PLID 26234 - Save the changes made to the preferred procedure list
void CConfigureNexEMRGroupsDlg::OnEditingFinishedConfigurePreferredProcList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		if (lpRow && bCommit) {
			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

			if (nCol == eppfCheckbox) {
				// Let's add this change to our list
				long nProcedureID = VarLong(pRow->GetValue(eppfID));
				PreferredProcChangeInfo ppciChangeInfo;
				if (m_mapPreferredProcChanges.Lookup(nProcedureID, ppciChangeInfo)) {
					// If a change entry already exists, then that means that the value has been set back to its original
					// value and a change entry is no longer necessary
					m_mapPreferredProcChanges.RemoveKey(nProcedureID);
				}
				else {
					// Add the change into to the list
					ppciChangeInfo.nProcedureID = nProcedureID;
					ppciChangeInfo.bChecked = VarBool(varNewValue);
					m_mapPreferredProcChanges[nProcedureID] = ppciChangeInfo;
				}
			}
		}
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnEditingFinishedConfigurePreferredProcList");	
}

// (a.wetta 2007-06-13 14:59) - PLID 26234 - Enable or disable the preferred procedure list when the checkbox is checked
void CConfigureNexEMRGroupsDlg::OnShowPreferredProc() 
{
	try {
		if (IsDlgButtonChecked(IDC_SHOW_PREFERRED_PROC)) {
			GetDlgItem(IDC_CONFIGURE_PREFERRED_PROC_LIST)->EnableWindow(TRUE);
		}
		else {
			GetDlgItem(IDC_CONFIGURE_PREFERRED_PROC_LIST)->EnableWindow(FALSE);
		}
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::OnShowPreferredProc");	
}

// (a.wetta 2007-06-13 14:59) - PLID 26234 - Make sure that the group sort orders are valid and fix them if need be
void CConfigureNexEMRGroupsDlg::VerifyTemplateSortOrders() 
{
	try {
		// Go through and make sure that the sort orders of all the items in the list are consecutive and that the first item starts at 0
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pEMNGroupTemplateList->GetFirstRow();
		long nFixSortOrder = 0;
		while (pRow) {
			long nCurSortOrder = VarLong(pRow->GetValue(egtfSortOrder));
			if (nCurSortOrder != nFixSortOrder) {
				pRow->PutValue(egtfSortOrder, _variant_t(nFixSortOrder));
				RecordEMNGroupTemplateChange(egtfSortOrder, pRow, _variant_t(nCurSortOrder), _variant_t(nFixSortOrder));
			}
			nFixSortOrder++;
			pRow = pRow->GetNextRow();
		}		
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::VerifyTemplateSortOrders");	
}

// (a.wetta 2007-06-13 14:59) - PLID 26234 - Make sure that the template sort orders are valid and fix them if need be
void CConfigureNexEMRGroupsDlg::VerifyGroupSortOrders() 
{
	try {
		// Go through and make sure that the sort orders of all the items in the list are consecutive and that the first item starts at 0
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pEMNGroupList->GetFirstRow();
		long nFixSortOrder = 0;
		while (pRow) {
			long nCurSortOrder = VarLong(pRow->GetValue(egfSortOrder));
			if (nCurSortOrder != nFixSortOrder) {
				pRow->PutValue(egfSortOrder, _variant_t(nFixSortOrder));
				RecordEMNGroupChange(egfSortOrder, pRow, _variant_t(nCurSortOrder), _variant_t(nFixSortOrder));
			}
			nFixSortOrder++;
			pRow = pRow->GetNextRow();
		}		
	}NxCatchAll("Error in CConfigureNexEMRGroupsDlg::VerifyGroupSortOrders");	
}

void CConfigureNexEMRGroupsDlg::OnRequeryFinishedEmnGroupList(short nFlags)
{
	try
	{
		// (z.manning 2009-08-11 16:39) - PLID 32989 - If one of the groups is set to auto-expand,
		// check the expand box for that row.
		long nExpandGroupID = GetRemotePropertyInt("EMRAutoExpandGroupID", -1, 0, GetCurrentUserName());
		if(nExpandGroupID != -1) {
			NXDATALIST2Lib::IRowSettingsPtr pExpandRow = m_pEMNGroupList->FindByColumn(egfID, nExpandGroupID, NULL, VARIANT_FALSE);
			if(pExpandRow != NULL) {
				pExpandRow->PutValue(egfExpand, g_cvarTrue);
			}
		}

		// (z.manning, 09/12/2007) - PLID 26234 - Select the top row.
		m_pEMNGroupList->PutCurSel(m_pEMNGroupList->GetTopRow());
		SelChangedEmnGroupsList(NULL, m_pEMNGroupList->GetCurSel());

	}NxCatchAll("CConfigureNexEMRGroupsDlg::OnRequeryFinishedEmnGroupList");
}
