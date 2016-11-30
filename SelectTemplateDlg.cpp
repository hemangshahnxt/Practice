// SelectTemplateDlg.cpp : implementation file
//

#include "stdafx.h"
#include "patientsrc.h"
#include "SelectTemplateDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSelectTemplateDlg dialog

CSelectTemplateDlg::CSelectTemplateDlg(CWnd* pParent)
	: CNxDialog(CSelectTemplateDlg::IDD, pParent)
{
	m_bAllowMultiple = true;
	m_nBehaviorType = btSingleProcedure;
	m_nLadderSelCount = 0;

	//{{AFX_DATA_INIT(CSelectTemplateDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CSelectTemplateDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectTemplateDlg)
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_SELECT_TEMPLATE_CAPTION, m_nxstaticSelectTemplateCaption);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectTemplateDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSelectTemplateDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CSelectTemplateDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CSelectTemplateDlg)
	ON_EVENT(CSelectTemplateDlg, IDC_TEMPLATE_NAME_LIST, 3 /* DblClickCell */, OnDblClickCellTemplateNameList, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSelectTemplateDlg message handlers

BOOL CSelectTemplateDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	try {
		// (c.haag 2008-05-01 10:39) - PLID 29866 - NxIconify buttons
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		m_mProcedureToLadder.RemoveAll();
		m_nLadderSelCount = 0;

		m_nBehaviorType = DetermineBehaviorType(m_rsTemplates);
		BuildLadderDataList(m_rsTemplates, m_nBehaviorType);

		switch (m_nBehaviorType) {
			case btSingleProcedure:
				BuildSingleProcedureControls();
				break;
			case btMultProceduresNoSeperateTracking:
				BuildMultipleProcedureControls(false);
				break;
			case btMultProceduresAllowSeperateTracking:
				BuildMultipleProcedureControls(true);
				break;
			case btPickOneLadderPerProcedure:
				BuildLadderCheckboxControls();
				break;
		}

		m_pTemplateNameList->CurSel = NULL;
	} NxCatchAll("Error In: CSelectTemplateDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectTemplateDlg::OnOK() 
{
	try {
		bool bValidSelection;
		if (m_nBehaviorType == btPickOneLadderPerProcedure) {
			bValidSelection = GetMultipleLadderSelection();
		} else {
			bValidSelection = GetSingleLadderSelection();
		}
		
		if (bValidSelection) {
			CDialog::OnOK();
		}
	} NxCatchAll("Error In: CSelectTemplateDlg::OnOK");
}

void CSelectTemplateDlg::OnCancel() 
{	
	try {
	CDialog::OnCancel();
	} NxCatchAll("Error In: CSelectTemplateDlg::OnCancel");
}

// (d.moore 2007-08-20) - PLID 14670 - Get the number of currently selected ladders.
long CSelectTemplateDlg::GetSelectionCount()
{
	return m_nLadderSelCount;
}

// (d.moore 2007-07-09 13:11) - PLID 14670 - Validate and store the selection for
//  behaviors allowing either single selection or the 'Track Separately' options.
bool CSelectTemplateDlg::GetSingleLadderSelection()
{
	m_nLadderSelCount = 0;
	m_mProcedureToLadder.RemoveAll();
	if (m_pTemplateNameList->CurSel == NULL) {
		MessageBox("You must select a ladder template from the list.");
		return false;
	}

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTemplateNameList->CurSel;
	long nProcedureID = VarLong(pRow->GetValue(ltcProcedureIdCol));
	long nLadderID = VarLong(pRow->GetValue(ltcLadderTemplateIdCol));
	if (nLadderID > 0) {
		// Only a single selection should be possible. Map every procedure
		//  to the same ladder template.
		m_nLadderSelCount = 1;
		pRow = m_pTemplateNameList->GetFirstRow();
		while (pRow != NULL) {
			nProcedureID = VarLong(pRow->GetValue(ltcProcedureIdCol));
			if (nProcedureID > 0) {
				m_mProcedureToLadder.SetAt(nProcedureID, nLadderID);
			}
			pRow = pRow->GetNextRow();
		}
	}
	else if (nLadderID == -1) {
		// 'Track Separately' option.
		pRow = m_pTemplateNameList->GetFirstRow();
		while (pRow != NULL) {
			nProcedureID = VarLong(pRow->GetValue(ltcProcedureIdCol));
			nLadderID = VarLong(pRow->GetValue(ltcLadderTemplateIdCol));
			if (nProcedureID > 0 && nLadderID > 0) {
				m_mProcedureToLadder.SetAt(nProcedureID, nLadderID);
				m_nLadderSelCount++;
			}
			pRow = pRow->GetNextRow();
		}
	}
	return true;
}

// (d.moore 2007-07-09 13:13) - PLID 14670 - Validate and store the selection for
//  behaviors that allow selection of ladders templates for each procedure.
bool CSelectTemplateDlg::GetMultipleLadderSelection()
{
	m_nLadderSelCount = 0;
	m_mProcedureToLadder.RemoveAll();
	BOOL bChecked;
	long nProcedureID, nLadderID, nValue;
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTemplateNameList->GetFirstRow();
	while (pRow != NULL) {
		bChecked = VarBool(pRow->GetValue(ltcCheckBoxCol), FALSE);
		if (bChecked) {
			nProcedureID = VarLong(pRow->GetValue(ltcProcedureIdCol));
			nLadderID = VarLong(pRow->GetValue(ltcLadderTemplateIdCol));
			if (m_mProcedureToLadder.Lookup(nProcedureID, nValue)) {
				// There is already a selection for this procedure.
				MessageBox("Please check only one ladder template per procedure.");
				m_mProcedureToLadder.RemoveAll();
				return false;
			} else {
				m_nLadderSelCount++;
				m_mProcedureToLadder.SetAt(nProcedureID, nLadderID);
			}
		}
		pRow = pRow->GetNextRow();
	}

	// There needs to be either one selection per category, or one selection only.
	if (m_mProcedureToLadder.GetCount() == 1) {
		return true;
	}
	else if (m_mProcedureToLadder.GetCount() == m_mProcedureCounts.GetCount()) {
		return true;
	}
	
	MessageBox("Please check one ladder template per procedure, or check "
		"just one ladder template to apply to all procedures in the list.");
	m_mProcedureToLadder.RemoveAll();
	return false;
}

/// (d.moore 2007-07-09 13:02) - PLID 14670 - Since it is now possible to select multiple
//  ladder templates in some cases, the selected IDs are now accessed by passing in a
//  CMap that will have the values copied into it.
void CSelectTemplateDlg::GetSelectedLadderIds(CMap<long, long, long, long> &arIDs)
{
	arIDs.RemoveAll();
	POSITION pos = m_mProcedureToLadder.GetStartPosition();
	long nKey, nVal;
	while (pos != NULL) {
		m_mProcedureToLadder.GetNextAssoc(pos, nKey, nVal);
		arIDs.SetAt(nKey, nVal);
	}
}

// (d.moore 2007-08-20) - PLID 14670 - It is sometimes much easier to just get
//  the ID selected when you know that there should only be one row selected.
long CSelectTemplateDlg::GetSelectedLadderID()
{
	// Return the first entry in the map.
	POSITION pos = m_mProcedureToLadder.GetStartPosition();
	long nKey;
	long nVal = -1;
	if (pos != NULL) {
		m_mProcedureToLadder.GetNextAssoc(pos, nKey, nVal);
	}
	return nVal;
}

// (d.moore 2007-07-06 09:08) - PLID 14670 - I've seperated out most of the functionality
//  for building the ladder template list. There are four different types of behaviour
//  for this dialog depending on the type of data that is passed in.

// (d.moore 2007-07-06 11:48) - PLID 14670 - Determine the correct behavior type. Returns
//  values that correspond to the EBehaviourTypes enum.
long CSelectTemplateDlg::DetermineBehaviorType(_RecordsetPtr rsLadderData)
{
	rsLadderData->MoveFirst();
	m_mProcedureCounts.RemoveAll();

	long nNumProcedures = 0;
	long nNumLadders = 0;
	long nNumEntries = rsLadderData->GetRecordCount();
	long nProcedureID = 0;
	long nTestID;

	while(!rsLadderData->eof) {
		// Count the number of distinct procedures.
		nTestID = AdoFldLong(rsLadderData, "ProcedureID", -1);
		if (nProcedureID != nTestID) {
			m_mProcedureCounts.SetAt(nProcedureID, nNumLadders);
			nProcedureID = nTestID;
			nNumProcedures++;
			nNumLadders = 0;
		}
		nNumLadders++;
		rsLadderData->MoveNext();
	}
	m_mProcedureCounts.SetAt(nProcedureID, nNumLadders);
	m_mProcedureCounts.RemoveKey(0);


	bool bMultipleProcedures = (nNumProcedures > 1);
	bool bMultipleLaddersPerProcedure = ((nNumEntries - nNumProcedures) > 0);
	
	// Determine which of the four behavious the dialog will use.
	//  1) Only one procedure. Pick one ladder, no seperate tracking. 
	//  2) Pick one ladder. Same for multiple procedures and multiple ladders.
	//  3) Seperate tracking, there is only one ladder per procedure.
	//  4) Seperate tracking, multiple ladders per procedure. Pick one ladder per procedure.
	if (!bMultipleProcedures) {
		return btSingleProcedure;
	}
	else if (!m_bAllowMultiple && bMultipleProcedures) {
		return btMultProceduresNoSeperateTracking;
	}
	else if (m_bAllowMultiple && bMultipleProcedures && !bMultipleLaddersPerProcedure) {
		return btMultProceduresAllowSeperateTracking;
	}
	else if (m_bAllowMultiple && bMultipleProcedures && bMultipleLaddersPerProcedure) {
		return btPickOneLadderPerProcedure;
	}
	
	// Default case:
	return btSingleProcedure;
}

// (d.moore 2007-07-06 09:22) - PLID 14670 - Fill the ladder template data list based
//  on the type of behavior for the dialog.
void CSelectTemplateDlg::BuildLadderDataList(_RecordsetPtr rsLadderData, long nBehaviorType)
{
	rsLadderData->MoveFirst();
	// (a.walling 2007-11-13 09:10) - PLID 28059 - Bad Bind; this is a DL2.
	m_pTemplateNameList = BindNxDataList2Ctrl(IDC_TEMPLATE_NAME_LIST, false);
	NXDATALIST2Lib::IRowSettingsPtr pRow;

	while(!rsLadderData->eof) {
		pRow = m_pTemplateNameList->GetNewRow();
		pRow->PutValue(ltcProcedureIdCol, AdoFldLong(rsLadderData, "ProcedureID", 0));
		pRow->PutValue(ltcProcedureNameCol, (_variant_t) AdoFldString(rsLadderData, "ProcedureName", ""));
		pRow->PutValue(ltcLadderTemplateIdCol, AdoFldLong(rsLadderData, "LadderTemplateID", 0));
		pRow->PutValue(ltcLadderTemplateNameCol, (_variant_t) AdoFldString(rsLadderData, "LadderTemplateName", ""));
		pRow->PutValue(ltcCheckBoxCol, false);
		m_pTemplateNameList->AddRowAtEnd(pRow, NULL);
		rsLadderData->MoveNext();
	}
}

// (d.moore 2007-07-06 10:12) - PLID 14670 - There should only be one procedure in the list.
//  Set the dialog up to allow only a single selection.
void CSelectTemplateDlg::BuildSingleProcedureControls()
{
	CString strLabel = 
		"Please select the ladder template you would like to use to "
		"track the procedure.";
	SetDlgItemText(IDC_SELECT_TEMPLATE_CAPTION, strLabel);
}

// (d.moore 2007-07-06 10:15) - PLID 14670 - There should only be one ladder per procedure.
//  Select one ladder or select 'Track Seperately' option.
void CSelectTemplateDlg::BuildMultipleProcedureControls(bool bAllowSeperateTracking)
{
	CString strLabel = 
		"The procedures you have selected may not all be associated with the same "
		"ladder template.\nPlease select the ladder template you would like to use to "
		"track these procedures. ";
	
	if (bAllowSeperateTracking) {
		strLabel += "Or select \"{Track Procedures Separately}\" to track "
			"each procedure on a different ladder.";

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTemplateNameList->GetNewRow();
		pRow->PutValue(ltcProcedureIdCol, (long)-1);
		pRow->PutValue(ltcProcedureNameCol, _bstr_t("{Track Procedures Separately}"));
		pRow->PutValue(ltcLadderTemplateIdCol, (long)-1);
		pRow->PutValue(ltcLadderTemplateNameCol, "");
		m_pTemplateNameList->AddRowBefore(pRow, m_pTemplateNameList->GetFirstRow());
	}
	
	SetDlgItemText(IDC_SELECT_TEMPLATE_CAPTION, strLabel);
}

// (d.moore 2007-07-06 10:16) - PLID 14670 - When there are multiple ladders for at least one
//  procedure, and m_bAllowMultiple is true, force the user to select one ladder per procedure.
void CSelectTemplateDlg::BuildLadderCheckboxControls()
{	
	// Make the checkboxes visible for this behaviour type only.
	NXDATALIST2Lib::IColumnSettingsPtr pCol = m_pTemplateNameList->GetColumn(ltcCheckBoxCol);
	long nStyle = pCol->GetColumnStyle();
	nStyle = nStyle | csWidthAuto;
	pCol->PutColumnStyle(nStyle);
	
	// Check any rows where there is only one ladder for the procedure.
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTemplateNameList->GetFirstRow();
	POSITION pos = m_mProcedureCounts.GetStartPosition();
	long nKey, nVal;
	while (pos != NULL) {
		m_mProcedureCounts.GetNextAssoc(pos, nKey, nVal);
		if (nVal == 1) {
			pRow = m_pTemplateNameList->FindByColumn(ltcProcedureIdCol, nKey, NULL, false);
			if (pRow != NULL) {
				pRow->PutValue(ltcCheckBoxCol, true);
			}
		}
	}
	
	CString strLabel = 
		"The procedures you have selected may not all be associated with the "
		"same ladder template. Please check the box next to each ladder you "
		"would like to use. Each procedure must have a ladder checked, and "
		"only one ladder can be selected per procedure. Or you can check just "
		"one ladder and have it apply to all of the procedures.";
	SetDlgItemText(IDC_SELECT_TEMPLATE_CAPTION, strLabel);
}

// (d.moore 2007-10-08) - PLID 14670 - Added the ability to double click on a row to select
//  it. Depending on the behavior selected for the dialog this may do different things.
void CSelectTemplateDlg::OnDblClickCellTemplateNameList(LPDISPATCH lpRow, short nColIndex) 
{
	switch (m_nBehaviorType) {
		case btSingleProcedure: 
			OnOK();
			break;
		case btMultProceduresNoSeperateTracking: 
			OnOK();
			break;
		case btMultProceduresAllowSeperateTracking: 
			OnOK();
			break;
		case btPickOneLadderPerProcedure: 
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pTemplateNameList->CurSel;
			if (pRow != NULL) {
				BOOL bChecked = VarBool(pRow->GetValue(ltcCheckBoxCol), FALSE);
				pRow->PutValue(ltcCheckBoxCol, !bChecked);
			}
			break;
	}
}
