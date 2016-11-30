// LabsToBeOrderedSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LabsToBeOrderedSetupDlg.h"
#include "EditComboBox.h"

// (j.jones 2010-06-25 16:01) - PLID 39185 - created

// CLabsToBeOrderedSetupDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

enum LocationComboColumns {

	lccID = 0,
	lccName,
};

enum ToBeOrderedListColumns {

	tbolcID = 0,
	tbolcOldLinked,
	tbolcNewLinked,
	tbolcDescription,
};

IMPLEMENT_DYNAMIC(CLabsToBeOrderedSetupDlg, CNxDialog)

CLabsToBeOrderedSetupDlg::CLabsToBeOrderedSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CLabsToBeOrderedSetupDlg::IDD, pParent)
{
	m_nLastSelLocationID = -1;
	m_nDefaultLocationID = -1;
}

CLabsToBeOrderedSetupDlg::~CLabsToBeOrderedSetupDlg()
{
}

void CLabsToBeOrderedSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_EDIT_TO_BE_ORDERED_LIST, m_btnEditToBeOrderedList);
}


BEGIN_MESSAGE_MAP(CLabsToBeOrderedSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_EDIT_TO_BE_ORDERED_LIST, OnBtnEditToBeOrderedList)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MESSAGE_MAP()


// CLabsToBeOrderedSetupDlg message handlers

BOOL CLabsToBeOrderedSetupDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnEditToBeOrderedList.AutoSet(NXB_MODIFY);

		m_Combo = BindNxDataList2Ctrl(IDC_LAB_TO_BE_ORDERED_LOCATION_COMBO);
		m_List = BindNxDataList2Ctrl(IDC_LAB_TO_BE_ORDERED_LIST, false);

		IRowSettingsPtr pRow = NULL;

		//do we have a default locationID?
		if(m_nDefaultLocationID != -1) {
			pRow = m_Combo->SetSelByColumn(lccID, m_nDefaultLocationID);
		}

		if(pRow == NULL) {
			//force the location requery to finish so content is displayed immediately
			m_Combo->WaitForRequery(dlPatienceLevelWaitIndefinitely);
			pRow = m_Combo->GetFirstRow();
		}

		if(pRow) {
			m_Combo->PutCurSel(pRow);
			
			m_nLastSelLocationID = VarLong(pRow->GetValue(lccID));

			LoadLocationList(m_nLastSelLocationID);
		}

	} NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CLabsToBeOrderedSetupDlg::OnBtnEditToBeOrderedList()
{
	try {

		IRowSettingsPtr pLocRow = m_Combo->GetCurSel();

		//see if something changed, if so warn that the changes will be saved first
		//(if no location is selected, there's nothing to save)
		if(pLocRow != NULL && HasSelectionsChanged()) {

			long nLocationID = VarLong(pLocRow->GetValue(lccID));

			if(IDYES == MessageBox("The changes you have made must be saved before editing the list. Do you wish to save now?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				if(!Save(nLocationID)) {
					//saving failed, cancel editing
					return;
				}
			}
			else {
				//don't edit
				return;
			}
		}

		// (j.armen 2012-06-06 15:51) - PLID 49856 - Refactored CEditComboBox
		CEditComboBox(this, 63, "Edit Labs To Be Ordered List").DoModal();

		//if no location is selected, there's nothing to show, so no need to requery
		if(pLocRow != NULL) {
			m_List->Requery();
		}

	} NxCatchAll(__FUNCTION__);
}

void CLabsToBeOrderedSetupDlg::OnOk()
{
	try {

		IRowSettingsPtr pLocRow = m_Combo->GetCurSel();

		//see if something changed, if so warn that the changes will be saved first
		//(if no location is selected, there's nothing to save)
		if(pLocRow != NULL && HasSelectionsChanged()) {

			long nLocationID = VarLong(pLocRow->GetValue(lccID));

			if(!Save(nLocationID)) {
				return;
			}
		}

		CNxDialog::OnOK();

	} NxCatchAll(__FUNCTION__);
}

void CLabsToBeOrderedSetupDlg::OnCancel()
{
	try {

		if(HasSelectionsChanged()) {
			if(IDNO == MessageBox("You have made changes to this setup, are you sure you wish to cancel?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				return;
			}
		}

		CNxDialog::OnCancel();

	} NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CLabsToBeOrderedSetupDlg, CNxDialog)
	ON_EVENT(CLabsToBeOrderedSetupDlg, IDC_LAB_TO_BE_ORDERED_LOCATION_COMBO, 16, OnSelChosenLabToBeOrderedLocationCombo, VTS_DISPATCH)
	ON_EVENT(CLabsToBeOrderedSetupDlg, IDC_LAB_TO_BE_ORDERED_LOCATION_COMBO, 1, OnSelChangingLabToBeOrderedLocationCombo, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

void CLabsToBeOrderedSetupDlg::OnSelChosenLabToBeOrderedLocationCombo(LPDISPATCH lpRow)
{
	try {

		IRowSettingsPtr pRow(lpRow);

		long nLocationID = -1;
		if(pRow) {
			nLocationID = VarLong(pRow->GetValue(lccID));
		}

		if(nLocationID == m_nLastSelLocationID) {
			//do nothing
			return;
		}

		//did we change locations?
		//if so, see if something changed, if so warn that the changes will be saved first
		if(nLocationID != m_nLastSelLocationID && HasSelectionsChanged()) {
			if(IDYES == MessageBox("The changes you have made must be saved before selecting another location. Do you wish to save now?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				if(!Save(m_nLastSelLocationID)) {
					//saving failed, reselect the previous location
					m_Combo->SetSelByColumn(lccID, m_nLastSelLocationID);
					return;
				}
			}
			else {
				//reselect the previous location
				m_Combo->SetSelByColumn(lccID, m_nLastSelLocationID);
				return;
			}
		}

		m_nLastSelLocationID = nLocationID;

		if(nLocationID == -1) {
			m_List->Clear();
			return;
		}
		
		LoadLocationList(m_nLastSelLocationID);

	} NxCatchAll(__FUNCTION__);
}

void CLabsToBeOrderedSetupDlg::OnSelChangingLabToBeOrderedLocationCombo(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {

		//force a selection
		if(*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	} NxCatchAll(__FUNCTION__);
}

void CLabsToBeOrderedSetupDlg::LoadLocationList(long nLocationID)
{
	try {

		CString strFromClause;
		strFromClause.Format("(SELECT ID, Description, "
			"Convert(bit, CASE WHEN ID IN (SELECT LabsToBeOrderedID FROM LabsToBeOrderedLocationLinkT WHERE LocationID = %li) THEN 1 ELSE 0 END) AS IsLinked "
			"FROM LabsToBeOrderedT) AS Query", nLocationID);
		m_List->PutFromClause(_bstr_t(strFromClause));
		m_List->Requery();

	}NxCatchAll(__FUNCTION__);
}

//returns TRUE if any linked checkbox doesn't match what is saved
BOOL CLabsToBeOrderedSetupDlg::HasSelectionsChanged()
{
	try {

		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while(pRow) {

			BOOL bOldValue = VarBool(pRow->GetValue(tbolcOldLinked));
			BOOL bNewValue = VarBool(pRow->GetValue(tbolcNewLinked));

			if(bOldValue != bNewValue) {
				return TRUE;
			}

			pRow = pRow->GetNextRow();
		}

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}

BOOL CLabsToBeOrderedSetupDlg::Save(long nLocationID)
{
	try {

		CWaitCursor pWait;

		CString strSqlBatch;

		IRowSettingsPtr pRow = m_List->GetFirstRow();
		while(pRow) {

			BOOL bOldValue = VarBool(pRow->GetValue(tbolcOldLinked));
			BOOL bNewValue = VarBool(pRow->GetValue(tbolcNewLinked));

			//only save if it changed
			if(bOldValue != bNewValue) {

				long nID = VarLong(pRow->GetValue(tbolcID));

				if(bNewValue) {
					//add a link, ensure there is no duplicate
					AddStatementToSqlBatch(strSqlBatch, "INSERT INTO LabsToBeOrderedLocationLinkT (LocationID, LabsToBeOrderedID) "
						"SELECT %li, ID FROM LabsToBeOrderedT "
						"WHERE ID = %li AND ID NOT IN (SELECT LabsToBeOrderedID FROM LabsToBeOrderedLocationLinkT WHERE LocationID = %li)",
						nLocationID, nID, nLocationID);
				}
				else {
					//remove the link
					AddStatementToSqlBatch(strSqlBatch, "DELETE FROM LabsToBeOrderedLocationLinkT "
						"WHERE LocationID = %li AND LabsToBeOrderedID = %li",
						nLocationID, nID);
				}
			}

			pRow = pRow->GetNextRow();
		}

		if(!strSqlBatch.IsEmpty()) {
			ExecuteSqlBatch(strSqlBatch);
		}

		//now update the "old" values
		pRow = m_List->GetFirstRow();
		while(pRow) {
			pRow->PutValue(tbolcOldLinked, pRow->GetValue(tbolcNewLinked));
			pRow = pRow->GetNextRow();
		}

		return TRUE;

	}NxCatchAll(__FUNCTION__);

	return FALSE;
}