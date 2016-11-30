// ProcedureLadderAssignment.cpp : implementation file
//

#include "stdafx.h"
#include "administratorRc.h"
#include "ProcedureLadderAssignment.h"
#include "GlobalDataUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcedureLadderAssignment dialog

// (d.moore 2007-08-24) - PLID 23497 - This dialog lets the user assign a ladder
//  to multiple procedures at once.

using namespace NXDATALIST2Lib;

//(e.lally 2010-07-28) PLID 36199 - Added a checkbox for the default selection
enum EColNames {
	ecnID=0,
	ecnName,
	ecnDefault,
};

CProcedureLadderAssignment::CProcedureLadderAssignment(CWnd* pParent)
	: CNxDialog(CProcedureLadderAssignment::IDD, pParent)
{
	//{{AFX_DATA_INIT(CProcedureLadderAssignment)
	//}}AFX_DATA_INIT
	m_nProcedureID = -1;
}


void CProcedureLadderAssignment::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcedureLadderAssignment)
	DDX_Control(pDX, IDC_STATIC_CONG_LAD_ASSING_SEL, m_lblSelected);
	DDX_Control(pDX, IDC_STATIC_CONF_LAD_ASSIGN_AVAIL, m_lblAvailable);
	DDX_Control(pDX, IDC_STATIC_CONF_LAD_ASSIGN_PROC, m_lblProcedure);
	DDX_Control(pDX, IDC_MOVE_PROC_RIGHT, m_btnMoveProcRight);
	DDX_Control(pDX, IDC_MOVE_PROC_LEFT, m_btnMoveProcLeft);
	DDX_Control(pDX, IDC_SEL_LADDER, m_btnSelLadder);
	DDX_Control(pDX, IDC_DESEL_LADDER, m_btnDeselLadder);
	DDX_Control(pDX, IDOK, m_btnOK);
	//}}AFX_DATA_MAP
}

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_MESSAGE_MAP(CProcedureLadderAssignment, CNxDialog)
	//{{AFX_MSG_MAP(CProcedureLadderAssignment)
	ON_BN_CLICKED(IDC_SEL_LADDER, OnSelLadder)
	ON_BN_CLICKED(IDC_DESEL_LADDER, OnDeselLadder)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_MOVE_PROC_LEFT, OnMoveProcLeft)
	ON_BN_CLICKED(IDC_MOVE_PROC_RIGHT, OnMoveProcRight)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcedureLadderAssignment message handlers

// (a.walling 2008-07-29 13:56) - PLID 30491 - Needs proper base class for message and event sink maps
BEGIN_EVENTSINK_MAP(CProcedureLadderAssignment, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CProcedureLadderAssignment)
	ON_EVENT(CProcedureLadderAssignment, IDC_DL_PROCEDURES, 18 /* RequeryFinished */, OnRequeryFinishedDlProcedures, VTS_I2)
	ON_EVENT(CProcedureLadderAssignment, IDC_DL_PROCEDURES, 2 /* SelChanged */, OnSelChangedDlProcedures, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CProcedureLadderAssignment, IDC_DL_AVAIL_LADDERS, 3 /* DblClickCell */, OnDblClickCellDlAvailLadders, VTS_DISPATCH VTS_I2)
	ON_EVENT(CProcedureLadderAssignment, IDC_DL_SELECTED_LADDERS, 3 /* DblClickCell */, OnDblClickCellDlSelectedLadders, VTS_DISPATCH VTS_I2)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CProcedureLadderAssignment, IDC_DL_SELECTED_LADDERS, 10, CProcedureLadderAssignment::EditingFinishedDlSelectedLadders, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CProcedureLadderAssignment, IDC_DL_SELECTED_LADDERS, 9, CProcedureLadderAssignment::EditingFinishingDlSelectedLadders, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

BOOL CProcedureLadderAssignment::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {
		m_pProcedureList = BindNxDataList2Ctrl(IDC_DL_PROCEDURES, false);
		m_pAvailLaderList = BindNxDataList2Ctrl(IDC_DL_AVAIL_LADDERS, false);
		m_pSelLadderList = BindNxDataList2Ctrl(IDC_DL_SELECTED_LADDERS, false);

		m_pProcedureList->Requery();
		
		m_btnMoveProcLeft.AutoSet(NXB_LEFT);
		m_btnMoveProcRight.AutoSet(NXB_RIGHT);
		m_btnDeselLadder.AutoSet(NXB_LEFT);
		m_btnSelLadder.AutoSet(NXB_RIGHT);
		m_btnOK.AutoSet(NXB_CLOSE);

		// Configure labels.
		m_lblProcedure.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_lblProcedure.SetText("Procedure:");

		m_lblAvailable.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_lblAvailable.SetText("Available Ladders:");
		
		m_lblSelected.SetColor(GetNxColor(GNC_ADMIN, 0));
		m_lblSelected.SetText("Selected Ladders:");
		
	} NxCatchAll("Error In: CProcedureLadderAssignment::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProcedureLadderAssignment::OnOK() 
{
	try {
		CDialog::OnOK();
	} NxCatchAll("Error In: CProcedureLadderAssignment::OnOK");
}

void CProcedureLadderAssignment::OnClose() 
{
	CDialog::OnCancel();
}

void CProcedureLadderAssignment::OnCancel() 
{
	CDialog::OnCancel();
}

void CProcedureLadderAssignment::SetSelectedProcedure(long nProcID)
{
	// This function allows a value to be passed in before the OnInitDialog
	//  function gets called.
	m_nProcedureID = nProcID;
}

long CProcedureLadderAssignment::GetSelectedProcedure()
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureList->CurSel;
	if (pRow != NULL) {
		long nProcID = VarLong(pRow->GetValue(ecnID));
	}
	return -1;
}

void CProcedureLadderAssignment::OnRequeryFinishedDlProcedures(short nFlags) 
{
	try {
		// Check to see that something got loaded in the list.
		long nNumRows = m_pProcedureList->GetRowCount();
		if (nNumRows <= 0) {
			EnableControls(FALSE);
			return;
		} else {
			EnableControls(TRUE);
			// If there is only one row in the list, then we don't need the 
			//  quick select buttons. So just disable them.
			if (nNumRows == 1) {
				GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(FALSE);
				GetDlgItem(IDC_MOVE_PROC_RIGHT)->EnableWindow(FALSE);
			}
		}
		
		// Now try to set a row selection.

		NXDATALIST2Lib::IRowSettingsPtr pRow;
		if (m_nProcedureID <= 0) {
			// If there has been no value set for the procedure ID then try to 
			//  get the first item in the list.
			pRow = m_pProcedureList->GetFirstRow();
			if (pRow != NULL) {
				GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(FALSE);
				m_nProcedureID = pRow->GetValue(ecnID);
				m_pProcedureList->PutCurSel(pRow);
				RequeryLadderLists();
				return;
			}
		}
		else {
			// An ID value has been set at some point. So use it to make a current
			//  selection.
			pRow = m_pProcedureList->FindByColumn(ecnID, m_nProcedureID, NULL, TRUE);
			if (pRow != NULL) {
				NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_pProcedureList->GetFirstRow();
				if (pRow->IsSameRow(pFirstRow)) {
					// If we are on the first row then disable the back button.
					GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(FALSE);
				}
				RequeryLadderLists();
				return;
			}
		}

		// We shouldn't get here unless there was an invalid procedure ID set, or
		//  the procedure dropdown was empty. So just clear out the two ladder lists.
		m_pAvailLaderList->Clear();
		m_pSelLadderList->Clear();

	} NxCatchAll("Error In: CProcedureLadderAssignment::OnRequeryFinishedDlProcedures");
}

void CProcedureLadderAssignment::EnableControls(BOOL bEnable)
{
	GetDlgItem(IDC_DL_AVAIL_LADDERS)->EnableWindow(bEnable);
	GetDlgItem(IDC_DL_SELECTED_LADDERS)->EnableWindow(bEnable);
	GetDlgItem(IDC_SEL_LADDER)->EnableWindow(bEnable);
	GetDlgItem(IDC_DESEL_LADDER)->EnableWindow(bEnable);
	GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(bEnable);
	GetDlgItem(IDC_MOVE_PROC_RIGHT)->EnableWindow(bEnable);
	
	if (!bEnable) {
		m_pAvailLaderList->Clear();
		m_pSelLadderList->Clear();
	}
}

void CProcedureLadderAssignment::RequeryLadderLists()
{
	if (m_nProcedureID > 0) {
		CString strAvailQuery, strSelQuery;
		
		strAvailQuery.Format(
			"ID NOT IN (SELECT DISTINCT LadderTemplateID "
					"FROM ProcedureLadderTemplateT "
					"WHERE ProcedureID = %li)", 
			m_nProcedureID);
		
		//(e.lally 2010-07-28) PLID 36199 - Filter it on procedureID now
		strSelQuery.Format(
			"ProcedureLadderTemplateT.ProcedureID = %li", 
			m_nProcedureID);

		m_pAvailLaderList->PutWhereClause(_bstr_t(strAvailQuery));
		m_pSelLadderList->PutWhereClause(_bstr_t(strSelQuery));

		m_pAvailLaderList->Requery();
		m_pSelLadderList->Requery();
	} else {
		m_pAvailLaderList->Clear();
		m_pSelLadderList->Clear();
	}
}

void CProcedureLadderAssignment::OnSelChangedDlProcedures(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	// A new procedure was selected in the dropdown. Update the 'available' and 'selected' lists.
	try {
		m_nProcedureID = -1;
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureList->CurSel;
		if (pRow != NULL) {
			m_nProcedureID = pRow->GetValue(ecnID);

			// Check to see if we are now at the start or end of the list.
			if (m_pProcedureList->GetRowCount() > 1) {
				NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_pProcedureList->GetFirstRow();
				NXDATALIST2Lib::IRowSettingsPtr pLastRow = m_pProcedureList->GetLastRow();
				if (pRow->IsSameRow(pFirstRow)) {
					GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(FALSE);
					GetDlgItem(IDC_MOVE_PROC_RIGHT)->EnableWindow(TRUE);
				}
				else if (pRow->IsSameRow(pLastRow)) {
					GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(TRUE);
					GetDlgItem(IDC_MOVE_PROC_RIGHT)->EnableWindow(FALSE);
				}
				else {
					GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(TRUE);
					GetDlgItem(IDC_MOVE_PROC_RIGHT)->EnableWindow(TRUE);
				}
			} else {
				GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(FALSE);
				GetDlgItem(IDC_MOVE_PROC_RIGHT)->EnableWindow(FALSE);
			}
		}

		RequeryLadderLists();
	} NxCatchAll("Error In: CProcedureLadderAssignment::OnSelChangedDlProcedures");
}

void CProcedureLadderAssignment::OnSelLadder() 
{
	// Shift any selected items out of the left hand 'available' window and into
	//  the right hand 'selected' window. This also updates the database as well.
	try {
		if (m_nProcedureID <= 0) {
			return;
		}
		
		CString strQuery, strItemQ;
		long nTempID;

		// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
		NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_pAvailLaderList->GetFirstSelRow();
		while (pCurSelRow != NULL) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = pCurSelRow;
			pCurSelRow = pCurSelRow->GetNextSelRow();

			nTempID = VarLong(pRow->GetValue(ecnID));
			strItemQ.Format(
				"INSERT INTO ProcedureLadderTemplateT "
					"(ProcedureID, LadderTemplateID) "
					"VALUES (%li, %li);\r\n", 
					m_nProcedureID, nTempID);
			strQuery += strItemQ;
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new index-less method
			m_pSelLadderList->TakeRowAddSorted(pRow);
		}

		if (strQuery.GetLength() > 0) {
			ExecuteSqlStd(strQuery);
		}
	} NxCatchAll("Error In: CProcedureLadderAssignment::OnSelLadder");
}

void CProcedureLadderAssignment::OnDeselLadder() 
{
	// Shift any selected items out of the right hand 'selected' window and into
	//  the left hand 'available' window. This also updates the database as well.
	try {
		if (m_nProcedureID <= 0) {
			return;
		}
		
		CString strQuery, strItemQ;
		long nTempID;

		// (b.cardillo 2008-07-15 16:27) - PLID 30741 - Changed this loop to use Get__SelRow() set of functions instead of Get__SelEnum()
		NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_pSelLadderList->GetFirstSelRow();
		while (pCurSelRow != NULL) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = pCurSelRow;
			pCurSelRow = pCurSelRow->GetNextSelRow();

			nTempID = VarLong(pRow->GetValue(ecnID));
			strItemQ.Format(
				"DELETE FROM ProcedureLadderTemplateT "
					"WHERE ProcedureID = %li "
						"AND LadderTemplateID = %li",
					m_nProcedureID, nTempID);
			strQuery += strItemQ;
			//(e.lally 2010-08-18) PLID 36199 - Ensure the checkbox is unchecked
			pRow->PutValue(ecnDefault, VARIANT_FALSE);
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new index-less method
			m_pAvailLaderList->TakeRowAddSorted(pRow);
		}

		if (strQuery.GetLength() > 0) {
			ExecuteSqlStd(strQuery);
		}
	} NxCatchAll("Error In: CProcedureLadderAssignment::OnDeselLadder");
}

void CProcedureLadderAssignment::OnMoveProcLeft() 
{
	// Move the procedure dropdown menu selection up one item.

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureList->CurSel;
	NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_pProcedureList->GetFirstRow();

	// If the first row is somehow NULL then something is seriously wrong.
	if (pFirstRow == NULL) {
		m_nProcedureID = -1;
		EnableControls(false);
		return;
	}

	// If there is no current selection, then set the selection to the
	//  first row in the list.
	if (pRow == NULL) {
		m_pProcedureList->PutCurSel(pFirstRow);
		m_nProcedureID = VarLong(pFirstRow->GetValue(ecnID));
		GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(FALSE);
		GetDlgItem(IDC_MOVE_PROC_RIGHT)->EnableWindow(TRUE);
		RequeryLadderLists();
		return;
	}

	// If there was a selection, then we need to check and see if we are already on
	//  then first row in the list.
	if (pRow->IsSameRow(pFirstRow)) {
		// We really don't need to do anything else, so just disable the button.
		GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(FALSE);
		GetDlgItem(IDC_MOVE_PROC_RIGHT)->EnableWindow(TRUE);
		return;
	}

	// If we have a selection, and we aren't on the first row, then we can move
	//  back one position in the list.
	pRow = pRow->GetPreviousRow();
	m_nProcedureID = VarLong(pRow->GetValue(ecnID));
	m_pProcedureList->PutCurSel(pRow);
	RequeryLadderLists();
	GetDlgItem(IDC_MOVE_PROC_RIGHT)->EnableWindow(TRUE);

	// Check to see if we have moved to the first row. If we have then we need
	//  to disable the back button.
	if (pRow->IsSameRow(pFirstRow)) {
		GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(FALSE);
	}
}

void CProcedureLadderAssignment::OnMoveProcRight() 
{
	// Move the procedure dropdown menu selection down one item.
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProcedureList->CurSel;
	NXDATALIST2Lib::IRowSettingsPtr pFirstRow = m_pProcedureList->GetFirstRow();
	NXDATALIST2Lib::IRowSettingsPtr pLastRow = m_pProcedureList->GetLastRow();

	// If the first or last row is somehow NULL then something is seriously wrong.
	if (pFirstRow == NULL || pLastRow == NULL) {
		m_nProcedureID = -1;
		EnableControls(false);
		return;
	}

	// If there is no current selection, then set the selection to the
	//  last row in the list.
	if (pRow == NULL) {
		m_pProcedureList->PutCurSel(pLastRow);
		m_nProcedureID = VarLong(pLastRow->GetValue(ecnID));
		GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(TRUE);
		GetDlgItem(IDC_MOVE_PROC_RIGHT)->EnableWindow(FALSE);
		RequeryLadderLists();
		return;
	}

	// If there was a selection, then we need to check and see if we are already on
	//  then first row in the list.
	if (pRow->IsSameRow(pLastRow)) {
		// We really don't need to do anything else, so just disable the button.
		GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(TRUE);
		GetDlgItem(IDC_MOVE_PROC_RIGHT)->EnableWindow(FALSE);
		return;
	}

	// If we have a selection, and we aren't on the last row, then we can move
	//  forward one position in the list.
	pRow = pRow->GetNextRow();
	m_nProcedureID = VarLong(pRow->GetValue(ecnID));
	m_pProcedureList->PutCurSel(pRow);
	RequeryLadderLists();
	GetDlgItem(IDC_MOVE_PROC_LEFT)->EnableWindow(TRUE);

	// Check to see if we have moved to the last row. If we have then we need
	//  to disable the forward button.
	if (pRow->IsSameRow(pLastRow)) {
		GetDlgItem(IDC_MOVE_PROC_RIGHT)->EnableWindow(FALSE);
	}
}

void CProcedureLadderAssignment::OnDblClickCellDlAvailLadders(LPDISPATCH lpRow, short nColIndex) 
{
	// If the user double clicks on an item in the 'available' list, then move it to
	//  the 'selected' list.
	try {
		if (m_nProcedureID <= 0) {
			return;
		}
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pAvailLaderList->CurSel;
		if (pRow == NULL) {
			return;
		}
		
		long nTempID = VarLong(pRow->GetValue(ecnID));

		//(e.lally 2010-07-28) PLID 36199 - Added default column, parameterized
		ExecuteParamSql("INSERT INTO ProcedureLadderTemplateT "
			"(ProcedureID, LadderTemplateID, [Default]) "
			"VALUES ({INT}, {INT}, 0);\r\n", 
				m_nProcedureID, nTempID);
		
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new index-less method
		m_pSelLadderList->TakeRowAddSorted(pRow);

	} NxCatchAll("Error In: CProcedureLadderAssignment::OnDblClickCellDlAvailLadders");
}

void CProcedureLadderAssignment::OnDblClickCellDlSelectedLadders(LPDISPATCH lpRow, short nColIndex) 
{
	// If the user double clicks on an item in the 'selected' list, then move it to
	//  the 'available' list.
	try {
		if (m_nProcedureID <= 0) {
			return;
		}
		
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pSelLadderList->CurSel;
		if (pRow == NULL) {
			return;
		}
		
		long nTempID;
		nTempID = VarLong(pRow->GetValue(ecnID));

		CString strQuery;
		strQuery.Format(
			"DELETE FROM ProcedureLadderTemplateT "
					"WHERE ProcedureID = %li "
						"AND LadderTemplateID = %li",
					m_nProcedureID, nTempID);
		
		ExecuteSqlStd(strQuery);
		//(e.lally 2010-08-18) PLID 36199 - Ensure the checkbox is unchecked
		pRow->PutValue(ecnDefault, VARIANT_FALSE);
		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to using the new index-less method
		m_pAvailLaderList->TakeRowAddSorted(pRow);

	} NxCatchAll("Error In: CProcedureLadderAssignment::OnDeselLadder");
}

//(e.lally 2010-07-28) PLID 36199
void CProcedureLadderAssignment::EditingFinishedDlSelectedLadders(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit)
{
	try {
		if(nCol == ecnDefault && bCommit){
			IRowSettingsPtr pRow(lpRow);
			if(pRow == NULL){
				return;
			}

			long nRowTemplateID = VarLong(pRow->GetValue(ecnID));
			BOOL bDefaultOn = AsBool(varNewValue);
			//ensure the other ladders are not set to be the default
			ExecuteParamSql("UPDATE ProcedureLadderTemplateT SET ProcedureLadderTemplateT.[Default] = CASE WHEN LadderTemplateID = {INT} THEN {BIT} ELSE CONVERT(BIT, 0) END WHERE ProcedureID = {INT} ",
				nRowTemplateID, bDefaultOn, m_nProcedureID);

			//Only allow one to be checked per procedure
			if(AsBool(varNewValue)){
				IRowSettingsPtr pCurRow = m_pSelLadderList->GetFirstRow();
				while(pCurRow){
					if(!AsBool(pCurRow->IsSameRow(pRow))){
						if(AsBool(pCurRow->GetValue(ecnDefault))){
							pCurRow->PutValue(ecnDefault, _variant_t(VARIANT_FALSE, VT_BOOL));
							return;
						}
					}
					pCurRow = pCurRow->GetNextRow();
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

//(e.lally 2010-07-28) PLID 36199
void CProcedureLadderAssignment::EditingFinishingDlSelectedLadders(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		if(nCol == ecnDefault && *pbCommit){
			IRowSettingsPtr pRow(lpRow);
			if(pRow == NULL){
				return;
			}
			IRowSettingsPtr pFirstSelRow = m_pSelLadderList->GetFirstSelRow();
			if(pFirstSelRow == NULL || pFirstSelRow->GetNextSelRow() != NULL){
				//we have multiple selections, so stop the commit
				*pbCommit = FALSE;
				return;
			}
		}
	}NxCatchAll(__FUNCTION__);
}
