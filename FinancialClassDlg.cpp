// FinancialClassDlg.cpp : implementation file
// (c.haag 2009-03-17 11:10) - PLID 9148 - Initial implementation
//
// This dialog lets users quickly assign multiple insurance companies
// to "financial classes", which are generic billing categories used
// for reporting purposes.
//
#include "stdafx.h"
#include "Practice.h"
#include "FinancialClassDlg.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"

// CFinancialClassDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



IMPLEMENT_DYNAMIC(CFinancialClassDlg, CNxDialog)

CFinancialClassDlg::CFinancialClassDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CFinancialClassDlg::IDD, pParent)
{
	m_clrStartup = RGB(0,0,0);
	m_bNeedToRequeryUnselectedList = TRUE;
}

CFinancialClassDlg::~CFinancialClassDlg()
{
}

void CFinancialClassDlg::SetStartupColor(const COLORREF& clr)
{
	// The edit insurance company background color is dynamic,
	// so we must be as well
	m_clrStartup = clr;
}

void CFinancialClassDlg::EnableControls(BOOL bEnable)
{
	// Controls are only disabled when there's nothing to choose
	// from in the financial class dropdown
	GetDlgItem(IDC_COMBO_FINCLASSES)->EnableWindow(bEnable);
	GetDlgItem(IDC_RENAME_CLASS)->EnableWindow(bEnable);
	GetDlgItem(IDC_DELETE_CLASS)->EnableWindow(bEnable);
	GetDlgItem(IDC_REMOVE_ALL_COMPANIES)->EnableWindow(bEnable);
	GetDlgItem(IDC_REMOVE_COMPANY)->EnableWindow(bEnable);
	GetDlgItem(IDC_ADD_COMPANY)->EnableWindow(bEnable);
	GetDlgItem(IDC_ADD_ALL_COMPANIES)->EnableWindow(bEnable);
	GetDlgItem(IDC_FINCLASS_UNSELECTED)->EnableWindow(bEnable);
	GetDlgItem(IDC_FINCLASS_SELECTED)->EnableWindow(bEnable);
}

void CFinancialClassDlg::LoadFirstAvailableFinancialClass()
{
	IRowSettingsPtr pRow = m_dlFinClasses->GetFirstRow();
	if (NULL != pRow) {
		// We found a financial class. Select it and load the
		// content.
		const long nID = VarLong(pRow->GetValue(0L));
		m_dlFinClasses->CurSel = pRow;
		Load(nID);
		EnableControls(TRUE);
	} else {
		// No records. Do nothing, and disable all the controls
		// and clear the lists out.
		EnableControls(FALSE);
		m_dlUnselected->Clear();
		m_dlSelected->Clear();
	}
}

BOOL CFinancialClassDlg::Load(long nID)
{
	// Requery the unselected list
	if (m_bNeedToRequeryUnselectedList) {
		m_dlUnselected->Requery();
		m_bNeedToRequeryUnselectedList = FALSE;
	}
	// Requery the selected list	
	CString strWhere;
	strWhere.Format("FinancialClassID = %li AND Archived = 0", nID);
	m_dlSelected->WhereClause = _bstr_t(strWhere);
	m_dlSelected->Requery();
	return TRUE;
}

void CFinancialClassDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddProcedureDlg)
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_NEW_CLASS, m_btnNewClass);
	DDX_Control(pDX, IDC_RENAME_CLASS, m_btnRenameClass);
	DDX_Control(pDX, IDC_DELETE_CLASS, m_btnDeleteClass);
	DDX_Control(pDX, IDC_REMOVE_ALL_COMPANIES, m_btnRemoveAll);
	DDX_Control(pDX, IDC_REMOVE_COMPANY, m_btnRemove);
	DDX_Control(pDX, IDC_ADD_COMPANY, m_btnAdd);
	DDX_Control(pDX, IDC_ADD_ALL_COMPANIES, m_btnAddAll);
	DDX_Control(pDX, IDC_FINANCIALCLR1, m_bkg);
	DDX_Control(pDX, IDC_FINANCIALCLR2, m_bkg2);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFinancialClassDlg, CNxDialog)
	ON_BN_CLICKED(IDC_NEW_CLASS, OnBnClickedNewClass)
	ON_BN_CLICKED(IDC_RENAME_CLASS, OnBnClickedRenameClass)
	ON_BN_CLICKED(IDC_DELETE_CLASS, OnBnClickedDeleteClass)
	ON_BN_CLICKED(IDC_REMOVE_ALL_COMPANIES, &CFinancialClassDlg::OnBnClickedRemoveAllCompanies)
	ON_BN_CLICKED(IDC_REMOVE_COMPANY, &CFinancialClassDlg::OnBnClickedRemoveCompany)
	ON_BN_CLICKED(IDC_ADD_COMPANY, &CFinancialClassDlg::OnBnClickedAddCompany)
	ON_BN_CLICKED(IDC_ADD_ALL_COMPANIES, &CFinancialClassDlg::OnBnClickedAddAllCompanies)
END_MESSAGE_MAP()


// CFinancialClassDlg message handlers

BOOL CFinancialClassDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		// Initialize backgrounds
		m_bkg.SetColor(m_clrStartup);
		m_bkg2.SetColor(m_clrStartup);

		// Iconize buttons
		m_btnNewClass.AutoSet(NXB_NEW);
		m_btnRenameClass.AutoSet(NXB_MODIFY);
		m_btnDeleteClass.AutoSet(NXB_DELETE);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnRemoveAll.AutoSet(NXB_LLEFT);
		m_btnRemove.AutoSet(NXB_LEFT);
		m_btnAdd.AutoSet(NXB_RIGHT);
		m_btnAddAll.AutoSet(NXB_RRIGHT);

		// Set up datalists
		m_dlFinClasses = BindNxDataList2Ctrl(IDC_COMBO_FINCLASSES);
		m_dlUnselected = BindNxDataList2Ctrl(IDC_FINCLASS_UNSELECTED, false);
		m_dlSelected = BindNxDataList2Ctrl(IDC_FINCLASS_SELECTED, false);

		// Now get the first financial class in data, and load it. We must have the combo
		// fully loaded first.
		m_dlFinClasses->WaitForRequery(dlPatienceLevelWaitIndefinitely);
		LoadFirstAvailableFinancialClass();
	}
	NxCatchAll("Error in CFinancialClassDlg::OnInitDialog");
	return TRUE;
}

// Called when a user wants to add a new financial class
void CFinancialClassDlg::OnBnClickedNewClass()
{
	try {
		CString strResult;
		BOOL bRetry = TRUE;
		while (bRetry) {
			int nResult = InputBoxLimited(this, "Please enter a name for the class", strResult, "",255,false,false,NULL);
			if (IDOK == nResult && !strResult.IsEmpty()) {
				// Check for duplicates
				_RecordsetPtr prsDup = CreateParamRecordset("SELECT ID FROM FinancialClassT WHERE NAME = {STRING}", strResult);
				if (!prsDup->eof) {
					MsgBox(MB_OK | MB_ICONERROR, "This name is in use by another Financial Class. Please choose another name.");
				} else {
					prsDup->Close();
					_RecordsetPtr prs = CreateParamRecordset(
						"SET NOCOUNT ON \r\n"
						"INSERT INTO FinancialClassT (Name) VALUES ({STRING});\r\n"
						"SET NOCOUNT OFF \r\n"
						"SELECT CONVERT(int, SCOPE_IDENTITY()) AS ID", strResult);
					if (!prs->eof) {
						// Add this to the combo
						const long nNewID = AdoFldLong(prs, "ID");
						IRowSettingsPtr pRow = m_dlFinClasses->GetNewRow();
						pRow->Value[0L] = nNewID;
						pRow->Value[1L] = _bstr_t(strResult);
						IRowSettingsPtr pNewRow = m_dlFinClasses->AddRowSorted(pRow, NULL);
						m_dlFinClasses->CurSel = pNewRow;
						// Audit
						AuditEvent(-1, "", BeginNewAuditEvent(), aeiFinancialClass, nNewID, "<Created>", strResult, aepMedium, aetCreated);
						// Now enable all the controls since we know we have at least one item in the list
						EnableControls(TRUE);
						// Now load the new row
						Load(nNewID);
					} else {
						// This should never happen
						ASSERT(FALSE);
					}
					// All done
					bRetry = FALSE;
				}
			} else {
				// User cancelled
				bRetry = FALSE;
			}
		}
	}
	NxCatchAll("Error in CFinancialClassDlg::OnBnClickedNewClass");
}

// Called when a user wants to rename a financial class
void CFinancialClassDlg::OnBnClickedRenameClass()
{
	try {
		IRowSettingsPtr pRow = m_dlFinClasses->CurSel;
		if (NULL == pRow) {
			ASSERT(FALSE); // This should never happen
			return;
		}
		const long nID = VarLong(pRow->GetValue(0L));
		const CString strOldName = VarString(pRow->GetValue(1L));
		CString strName = VarString(pRow->GetValue(1L));

		BOOL bRetry = TRUE;
		while (bRetry) {
			int nResult = InputBoxLimited(this, "Please enter a name for the class", strName, "",255,false,false,NULL);
			if (IDOK == nResult && !strName.IsEmpty()) {
				// Check for duplicates
				_RecordsetPtr prs = CreateParamRecordset("SELECT ID FROM FinancialClassT WHERE NAME = {STRING} AND ID <> {INT}", strName, nID);
				if (!prs->eof) {
					MsgBox(MB_OK | MB_ICONERROR, "This name is in use by another Financial Class. Please choose another name.");
				} else {
					prs->Close();
					ExecuteParamSql("UPDATE FinancialClassT SET Name = {STRING} WHERE ID = {INT}", strName, nID);
					// Audit
					AuditEvent(-1, "", BeginNewAuditEvent(), aeiFinancialClass, nID, strOldName, strName, aepMedium, aetChanged);
					// Update the combo
					pRow->PutValue(1L, _bstr_t(strName));
					// All done
					bRetry = FALSE;
				}
			} else {
				// User cancelled out
				bRetry = FALSE;
			}
		}
	}
	NxCatchAll("Error in CFinancialClassDlg::OnBnClickedRenameClass");
}

// Called when a user wants to delete the selected financial class and
// unassign all companies from it
void CFinancialClassDlg::OnBnClickedDeleteClass()
{
	try {
		IRowSettingsPtr pRow = m_dlFinClasses->CurSel;
		if (NULL == pRow) {
			ASSERT(FALSE); // This should never happen
			return;
		} 

		// Calculate how many are in use from the list. We'll use it to form the warning message.
		long nUses = m_dlSelected->GetRowCount();
		if (nUses == 0) {
			if (IDNO == MsgBox(MB_ICONQUESTION | MB_YESNO, "Are you sure you wish to delete this class?")) {
				return;
			}
		}
		else if (nUses == 1) {
			if (IDNO == MsgBox(MB_ICONQUESTION | MB_YESNO, "There is currently one company assigned to this class. Are you sure you wish to delete this class?")) {
				return;
			}
		}
		else {
			if (IDNO == MsgBox(MB_ICONQUESTION | MB_YESNO, "There are currently %d companies assigned to this class. Are you sure you wish to delete this class?", nUses)) {
				return;
			}
		}

		const long nID = VarLong(pRow->GetValue(0L));
		const CString strName = VarString(pRow->GetValue(1L));
		ExecuteParamSql("UPDATE InsuranceCoT SET FinancialClassID = NULL WHERE FinancialClassID = {INT}\r\n"
			"DELETE FROM FinancialClassT WHERE ID = {INT}", nID, nID);
		// Audit
		AuditEvent(-1, "", BeginNewAuditEvent(), aeiFinancialClass, nID, strName, "<Deleted>", aepMedium, aetDeleted);

		// Remove the row from the list
		m_dlFinClasses->RemoveRow(pRow);

		// Now get the first financial class in data, and load it
		m_bNeedToRequeryUnselectedList = TRUE;
		LoadFirstAvailableFinancialClass();
	}
	NxCatchAll("Error in CFinancialClassDlg::OnBnClickedDeleteClass");
}

// Called when a user wants to remove all insurance companies from the
// selected financial class
void CFinancialClassDlg::OnBnClickedRemoveAllCompanies()
{
	try {
		IRowSettingsPtr pRow = m_dlSelected->GetFirstRow();
		if (NULL != pRow) {
			CSqlBatch bt;
			CAuditTransaction at;
			IRowSettingsPtr pFinClassRow = m_dlFinClasses->CurSel;
			const CString strFinClass = VarString(pFinClassRow->GetValue(1L));

			while (NULL != pRow) {
				const long nInsCoID = VarLong(pRow->GetValue(0L));
				const CString strName = VarString(pRow->GetValue(1L));
				bt.Add("UPDATE InsuranceCoT SET FinancialClassID = NULL WHERE PersonID = %d", nInsCoID);
				// Audit
				AuditEvent(-1, strName, at, aeiInsCoFinancialClass, nInsCoID, strFinClass, "", aepMedium, aetChanged);
				pRow = pRow->GetNextRow();
			}
			
			// Commit changes to data
			bt.Execute();
			at.Commit();

			m_dlUnselected->TakeAllRows(m_dlSelected);
		}		
	}
	NxCatchAll("Error in CFinancialClassDlg::OnBnClickedRemoveAllCompanies");
}

// Called when the user wants to move certain companies away from the
// selected financial class
void CFinancialClassDlg::OnBnClickedRemoveCompany()
{
	try {
		if (NULL != m_dlSelected->GetFirstSelRow()) {
			CSqlBatch bt;
			CAuditTransaction at;
			IRowSettingsPtr pFinClassRow = m_dlFinClasses->CurSel;
			const CString strFinClass = VarString(pFinClassRow->GetValue(1L));

			NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_dlSelected->GetFirstSelRow();
			while (pCurSelRow != NULL) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = pCurSelRow;
				pCurSelRow = pCurSelRow->GetNextSelRow();
				const long nInsCoID = VarLong(pRow->GetValue(0L));
				const CString strName = VarString(pRow->GetValue(1L));
				bt.Add("UPDATE InsuranceCoT SET FinancialClassID = NULL WHERE PersonID = %d", nInsCoID);
				// Audit
				AuditEvent(-1, strName, at, aeiInsCoFinancialClass, nInsCoID, strFinClass, "", aepMedium, aetChanged);
				m_dlUnselected->TakeRowAddSorted(pRow);
			}

			// Commit changes to data
			bt.Execute();
			at.Commit();	
		}		
	}
	NxCatchAll("Error in CFinancialClassDlg::OnBnClickedRemoveCompany");
}

// Called when a user want to assign a company to the selected financial class
void CFinancialClassDlg::OnBnClickedAddCompany()
{
	try {
		if (NULL != m_dlUnselected->GetFirstSelRow()) {
			CSqlBatch bt;
			CAuditTransaction at;
			IRowSettingsPtr pFinClassRow = m_dlFinClasses->CurSel;
			const CString strFinClass = VarString(pFinClassRow->GetValue(1L));
			const long nFinClassID = VarLong(pFinClassRow->GetValue(0L));

			NXDATALIST2Lib::IRowSettingsPtr pCurSelRow = m_dlUnselected->GetFirstSelRow();
			while (pCurSelRow != NULL) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = pCurSelRow;
				pCurSelRow = pCurSelRow->GetNextSelRow();

				const long nInsCoID = VarLong(pRow->GetValue(0L));
				const CString strName = VarString(pRow->GetValue(1L));
				bt.Add("UPDATE InsuranceCoT SET FinancialClassID = %d WHERE PersonID = %d", nFinClassID, nInsCoID);
				// Audit
				AuditEvent(-1, strName, at, aeiInsCoFinancialClass, nInsCoID, "", strFinClass, aepMedium, aetChanged);
				m_dlSelected->TakeRowAddSorted(pRow);
			}
			
			bt.Execute();
			at.Commit();	
		}		
	}
	NxCatchAll("Error in CFinancialClassDlg::OnBnClickedAddCompany");
}

// Called when a user want to assign all unassigned companies to the 
// selected financial class
void CFinancialClassDlg::OnBnClickedAddAllCompanies()
{
	try {
		IRowSettingsPtr pRow = m_dlUnselected->GetFirstRow();
		if (NULL != pRow) {
			CSqlBatch bt;
			CAuditTransaction at;
			IRowSettingsPtr pFinClassRow = m_dlFinClasses->CurSel;
			const long nFinClassID = VarLong(pFinClassRow->GetValue(0L));
			const CString strFinClass = VarString(pFinClassRow->GetValue(1L));

			while (NULL != pRow) {
				const long nInsCoID = VarLong(pRow->GetValue(0L));
				const CString strName = VarString(pRow->GetValue(1L));
				bt.Add("UPDATE InsuranceCoT SET FinancialClassID = %d WHERE PersonID = %d", nFinClassID, nInsCoID);
				// Audit
				AuditEvent(-1, strName, at, aeiInsCoFinancialClass, nInsCoID, "", strFinClass, aepMedium, aetChanged);
				pRow = pRow->GetNextRow();
			}
			
			bt.Execute();
			at.Commit();

			m_dlSelected->TakeAllRows(m_dlUnselected);
		}
	}
	NxCatchAll("Error in CFinancialClassDlg::OnBnClickedAddAllCompanies");
}
BEGIN_EVENTSINK_MAP(CFinancialClassDlg, CNxDialog)
	ON_EVENT(CFinancialClassDlg, IDC_COMBO_FINCLASSES, 1, CFinancialClassDlg::SelChangingComboFinclasses, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CFinancialClassDlg, IDC_COMBO_FINCLASSES, 16, CFinancialClassDlg::SelChosenComboFinclasses, VTS_DISPATCH)
	ON_EVENT(CFinancialClassDlg, IDC_FINCLASS_UNSELECTED, 3, CFinancialClassDlg::DblClickCellFinclassUnselected, VTS_DISPATCH VTS_I2)
	ON_EVENT(CFinancialClassDlg, IDC_FINCLASS_SELECTED, 3, CFinancialClassDlg::DblClickCellFinclassSelected, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CFinancialClassDlg::SelChangingComboFinclasses(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			// Don't let them select nothing, change it back to the old row
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll("Error in CFinancialClassDlg::SelChangingComboFinclasses");
}

// Called when the user changes the selected financial class
void CFinancialClassDlg::SelChosenComboFinclasses(LPDISPATCH lpRow)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow) {
			const long nID = VarLong(pRow->GetValue(0L));
			Load(nID);
		}
	}
	NxCatchAll("Error in CFinancialClassDlg::SelChosenComboFinclasses");
}

// Let users double-click on companies to add them to a financial class
void CFinancialClassDlg::DblClickCellFinclassUnselected(LPDISPATCH lpRow, short nColIndex)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow) {
			OnBnClickedAddCompany();
		}
	}
	NxCatchAll("Error in CFinancialClassDlg::DblClickCellFinclassUnselected");
}

// Let users double-click on companies to remove them from a financial class
void CFinancialClassDlg::DblClickCellFinclassSelected(LPDISPATCH lpRow, short nColIndex)
{
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow) {
			OnBnClickedRemoveCompany();
		}
	}
	NxCatchAll("Error in CFinancialClassDlg::DblClickCellFinclassSelected");
}
