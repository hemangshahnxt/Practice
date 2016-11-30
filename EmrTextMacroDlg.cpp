// EmrTextMacroDlg.cpp : implementation file
//
//
// (c.haag 2008-06-12 10:24) - PLID 26038 - Initial implementation
//

#include "stdafx.h"
#include "EmrTextMacroDlg.h"
#include "EmrItemAdvTextDlg.h"
#include "GlobalAuditUtils.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-04-06 13:51) - PLID 23643 - inappropriate command ID range (0x8000 -> 0xDFFF / 32768 -> 57343)
#define ID_DELETE_MACRO		50000

#define NEW_MACRO_COLOR		RGB(200,200,255)

#define SENTINEL_NEW_NAME	" <New Macro>"

using namespace NXDATALIST2Lib;
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



typedef enum {
	eclID,
	eclDetailName,
	eclTextData,
	eclModified
} EListColumns;

/////////////////////////////////////////////////////////////////////////////
// CEmrTextMacroDlg dialog


CEmrTextMacroDlg::CEmrTextMacroDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrTextMacroDlg::IDD, pParent)
	, m_wndPreview(this)
{
	//{{AFX_DATA_INIT(CEmrTextMacroDlg)
	m_strDetailName = _T("");
	//}}AFX_DATA_INIT
	m_bIgnoreEditChanges = FALSE;
}

BOOL CEmrTextMacroDlg::IsNewRow(IRowSettingsPtr& pRow)
{
	const long nID = VarLong(pRow->Value[eclID]);
	return (-1 == nID) ? TRUE : FALSE;
}

BOOL CEmrTextMacroDlg::IsContentEmpty(IRowSettingsPtr& pRow)
{
	CString strDetailName = VarString(pRow->Value[eclDetailName]);
	CString strTextData = VarString(pRow->Value[eclTextData]);
	if ( (strDetailName.IsEmpty() || strDetailName == SENTINEL_NEW_NAME) &&
		strTextData.IsEmpty()) {
		return TRUE;
	} else {
		return FALSE;
	}
}

void CEmrTextMacroDlg::CreateAndSelectEmptyRow()
{
	IRowSettingsPtr pRow = m_dlMacroList->GetNewRow();
	pRow->Value[eclID] = (long)-1;
	pRow->Value[eclDetailName] = SENTINEL_NEW_NAME;
	pRow->Value[eclTextData] = "";
	pRow->BackColor = NEW_MACRO_COLOR;
	m_dlMacroList->CurSel = m_dlMacroList->AddRowSorted(pRow, NULL);
	UpdateAppearance(pRow);
	// Reset our defaults
	m_strOldDetailName.Empty();
	m_strOldTextData.Empty();
	// Update the button enabled states
	UpdateButtons(pRow);
	// Now set the focus to the macro name
	GetDlgItem(IDC_EDIT_DETAIL_NAME)->SetFocus();
}

void CEmrTextMacroDlg::UpdateAppearance(IRowSettingsPtr& pRow)
{
	m_bIgnoreEditChanges = TRUE;
	if (NULL == pRow) {
		// This should not happen
		ThrowNxException("UpdateAppearance was called for an invalid row");
	}

	if (IsNewRow(pRow) && VarString(pRow->Value[eclDetailName]) == SENTINEL_NEW_NAME) {
		m_wndPreview.SetLabelText("");
		SetDlgItemText(IDC_EDIT_DETAIL_NAME, "");
	} else {
		m_wndPreview.SetLabelText(VarString(pRow->Value[eclDetailName]));
		SetDlgItemText(IDC_EDIT_DETAIL_NAME, VarString(pRow->Value[eclDetailName]));
	}
	m_wndPreview.SetTextData(VarString(pRow->Value[eclTextData]));

	// Update the button enabled states
	UpdateButtons(pRow);

	m_bIgnoreEditChanges = FALSE;
}

void CEmrTextMacroDlg::UpdateButtons(IRowSettingsPtr& pSel)
{
	const BOOL bIsNewRow = IsNewRow(pSel);

	// Update the Delete button. We can only delete rows that exist from data, so
	// rule out New rows.
	GetDlgItem(IDC_BTN_DELETE_MACRO)->EnableWindow(bIsNewRow ? FALSE : TRUE);

	// Update the Save and Add New button. This only applies to when the new row is
	// selected. The reason is that you can't add a new, unsaved row to the list if
	// a new, unsaved row already exists.
	GetDlgItem(IDC_BTN_NEW_MACRO)->EnableWindow(bIsNewRow ? TRUE : FALSE);
}

BOOL CEmrTextMacroDlg::NeedToSave()
{
	IRowSettingsPtr pRow = m_dlMacroList->CurSel;
	if (NULL == pRow) {
		return FALSE; // No row selected; nothing to do
	}

	// Compare each individual field
	if (VarString(pRow->Value[eclDetailName]) != m_strOldDetailName) {
		return TRUE;
	}
	if (VarString(pRow->Value[eclTextData]) != m_strOldTextData) {
		return TRUE;
	}

	// If we get here, everything matches
	return FALSE;
}

// (c.haag 2008-06-26 17:19) - Added a flag for querying data for validation
BOOL CEmrTextMacroDlg::ValidateContent(BOOL bCheckData)
{
	IRowSettingsPtr pRow = m_dlMacroList->CurSel;
	if (NULL == pRow) {
		ThrowNxException("No row was selected during content validation!");
	}

	// (a.walling 2010-12-30 14:30) - PLID 41961 - Automatically create a name from the content if none is specified.
	CString strDetailName = VarString(pRow->Value[eclDetailName], "");
	CString strTextData = VarString(pRow->Value[eclTextData], "");

	// Validate the detail name (no need to check size; the dialog already locks that to 255). We accept
	// everything but empty strings and the sentinel new macro name.
	if (strDetailName.IsEmpty() || strDetailName == SENTINEL_NEW_NAME) {
		if (strTextData.IsEmpty()) {
			MessageBox("You must enter a valid name for the current macro.", "NexTech Practice", MB_OK | MB_ICONERROR);
			GetDlgItem(IDC_EDIT_DETAIL_NAME)->SetFocus();
			return FALSE;
		} else {
			int autoIndex = strTextData.FindOneOf("\r\n");
			if (autoIndex == -1 || autoIndex > 255) {
				autoIndex = min(255, strTextData.GetLength());
			}
			strDetailName = strTextData.Left(autoIndex);
			SetDlgItemText(IDC_EDIT_DETAIL_NAME, strDetailName); // will also update the combo box and datalist in the change handler
		}
	}

	// (c.haag 2008-06-26 17:11) - PLID 26038 - Check for duplicates in data
	if (bCheckData) {
		long nID = VarLong(pRow->Value[eclID]);
		_RecordsetPtr prs = CreateParamRecordset("SELECT TOP 1 ID FROM EMRTextMacroT WHERE Name = {STRING} AND TextData = {STRING} AND ID <> {INT}",
			strDetailName, strTextData, nID);
		if (!prs->eof) {
			MessageBox("There is already an EMR text macro in your data which has the same name and content.", "NexTech Practice", MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}

	// Return TRUE meaning the validation did not fail
	return TRUE;
}

void CEmrTextMacroDlg::SaveContent()
{
	// (c.haag 2008-06-16 09:52) - This function saves the current macro.
	IRowSettingsPtr pRow = m_dlMacroList->CurSel;
	if (NULL == pRow) {
		ThrowNxException("No row was selected during content saving!");
	}
	long nAuditTransactionID = -1;
	long nID = VarLong(pRow->Value[eclID]);

	try {
		if (-1 == nID) {
			// Add a new row
			nAuditTransactionID = BeginAuditTransaction();
			_RecordsetPtr prs = CreateParamRecordset(
				"SET NOCOUNT ON\r\n"
				"INSERT INTO EMRTextMacroT "
				"(Name, TextData) "  
				"VALUES "
				"({STRING}, {STRING}) \r\n"
				"SET NOCOUNT OFF\r\n"
				"SELECT CONVERT(int, SCOPE_IDENTITY()) AS ID"
				,
				VarString(pRow->Value[eclDetailName]),
				VarString(pRow->Value[eclTextData])
				);
			_variant_t v = prs->Fields->Item["ID"]->Value;
			pRow->Value[eclID] = v;
			pRow->BackColor = RGB(255,255,255);

			const CString strNewValue = FormatString("'%s' - '%s'", 
				VarString(pRow->Value[eclDetailName]), VarString(pRow->Value[eclTextData]));
			AuditEvent(-1, "", nAuditTransactionID, aeiEMRTextMacroCreated, VarLong(v), "", strNewValue, aepMedium, aetCreated);
			CommitAuditTransaction(nAuditTransactionID);

		} else {
			// Update an existing row
			nAuditTransactionID = BeginAuditTransaction();
			ExecuteParamSql("UPDATE EMRTextMacroT SET "
				"Name = {STRING}, TextData = {STRING} "
				"WHERE ID = {INT} ",
				VarString(pRow->Value[eclDetailName]),
				VarString(pRow->Value[eclTextData]),
				nID);

			const CString strOldValue = FormatString("'%s' - '%s'", 
				m_strOldDetailName, m_strOldTextData);
			const CString strNewValue = FormatString("'%s' - '%s'", 
				VarString(pRow->Value[eclDetailName]), VarString(pRow->Value[eclTextData]));
			AuditEvent(-1, "", nAuditTransactionID, aeiEMRTextMacroChanged, nID, strOldValue, strNewValue, aepMedium, aetChanged);
			CommitAuditTransaction(nAuditTransactionID);
		}
	}
	catch (...) {
		if (-1 != nAuditTransactionID) {
			RollbackAuditTransaction(nAuditTransactionID);
		}
		throw;
	}
}

void CEmrTextMacroDlg::HandleListSelectionChange(IRowSettingsPtr& pRow)
{
	if (NULL != pRow) {
		m_strOldDetailName = VarString(pRow->Value[eclDetailName]);
		m_strOldTextData = VarString(pRow->Value[eclTextData]);
	} else {
		m_strOldDetailName.Empty();
		m_strOldTextData.Empty();
	}
	UpdateAppearance(pRow);
}

int CEmrTextMacroDlg::SaveOrDiscardCurrentSelection()
{
	// If this is the newly added macro, do nothing
	IRowSettingsPtr pRow(m_dlMacroList->CurSel);
	if (IsNewRow(pRow)) {
		return IDNO;
	}
	int nResult = IDYES;
	if (NeedToSave()) {
		switch (nResult = MessageBox("Do you wish to save your current macro?\r\n\r\nClicking 'No' will discard your changes.", "NexTech Practice", MB_YESNOCANCEL | MB_ICONQUESTION)) {
		case IDCANCEL:
			break; // User changed their mind
		case IDYES:
			SaveContent();
			break;
		case IDNO:
			// Restore the old values in the list
			pRow->Value[eclDetailName] = _bstr_t(m_strOldDetailName);
			pRow->Value[eclTextData] = _bstr_t(m_strOldTextData);
		default:
			break;
		}
	}
	return nResult;
}


void CEmrTextMacroDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrTextMacroDlg)
	DDX_Control(pDX, IDC_EDIT_DETAIL_NAME, m_nxeditDetailName);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BTN_NEW_MACRO, m_btnNewMacro);
	DDX_Control(pDX, IDC_BTN_DELETE_MACRO, m_btnDeleteMacro);
	DDX_Control(pDX, IDC_BTN_ADD_AND_SAVE, m_btnAddAndSave);
	DDX_Control(pDX, IDC_STATIC_PREVIEW, m_nxstaticPreview);
	DDX_Text(pDX, IDC_EDIT_DETAIL_NAME, m_strDetailName);
	DDV_MaxChars(pDX, m_strDetailName, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrTextMacroDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrTextMacroDlg)
	ON_BN_CLICKED(IDC_BTN_ADD_AND_SAVE, OnBtnAddAndSave)
	ON_EN_CHANGE(IDC_EDIT_DETAIL_NAME, OnChangeEditDetailName)
	ON_MESSAGE(NXM_EMR_TEXT_MACRO_BOX_EDIT_CHANGE, OnChangePreviewEdit)
	ON_BN_CLICKED(IDC_BTN_NEW_MACRO, OnBtnNewMacro)
	ON_BN_CLICKED(IDC_BTN_DELETE_MACRO, OnBtnDeleteMacro)
	ON_COMMAND(ID_DELETE_MACRO, OnPopupDeleteMacro)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrTextMacroDlg message handlers

BOOL CEmrTextMacroDlg::OnInitDialog() 
{	
	try {
		CNxDialog::OnInitDialog();

		// Bind datalists
		m_dlMacroList = BindNxDataList2Ctrl(IDC_EMR_TEXT_MACRO_LIST);

		// Set button styles
		m_btnCancel.AutoSet(NXB_CLOSE);
		m_btnNewMacro.AutoSet(NXB_NEW);
		m_btnDeleteMacro.AutoSet(NXB_DELETE);
		m_btnAddAndSave.AutoSet(NXB_OK);

		// Create and size the preview window
		CRect rcStatic, rcColor;
		m_nxstaticPreview.GetWindowRect(&rcStatic);
		ScreenToClient(&rcStatic);
		GetDlgItem(IDC_COMM_MACRO_COLOR)->GetWindowRect(&rcColor);
		ScreenToClient(&rcColor);
		m_wndPreview.Create(IDD_EMR_TEXT_MACRO_BOX, this);
		m_wndPreview.SetWindowPos(&wndTop, rcColor.left + 10, rcStatic.bottom + 5,
			353, 393, SWP_SHOWWINDOW);

		// Set the background colors
		((CNxColor*)GetDlgItem(IDC_COMM_MACRO_COLOR))->SetColor(GetNxColor(GNC_ADMIN, 0));
		((CNxColor*)GetDlgItem(IDC_COMM_MACRO_LIST_COLOR))->SetColor(GetNxColor(GNC_ADMIN, 0));

		// By default, the user is creating a new macro. So, it must have an entry in the datalist.
		// Create it, and select it.
		CreateAndSelectEmptyRow();
	}
	NxCatchAll("Error in CEmrTextMacroDlg::OnInitDialog");
	
	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEmrTextMacroDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrTextMacroDlg)
	ON_EVENT(CEmrTextMacroDlg, IDC_EMR_TEXT_MACRO_LIST, 3 /* DblClickCell */, OnDblClickCellEmrTextMacroList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CEmrTextMacroDlg, IDC_EMR_TEXT_MACRO_LIST, 1 /* SelChanging */, OnSelChangingEmrTextMacroList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CEmrTextMacroDlg, IDC_EMR_TEXT_MACRO_LIST, 7 /* RButtonUp */, OnRButtonUpEmrTextMacroList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEmrTextMacroDlg::OnDblClickCellEmrTextMacroList(LPDISPATCH lpRow, short nColIndex) 
{
	try {
		// If the user double-clicks on a list entry, treat is as if the user wanted to add it to
		// the EMR. We also want to save the selection as some users may adopt a habit of entering
		// a new macro and double-clicking it on the list...or maybe they edit an existing macro and
		// double-click assuming it will be saved. We can always make this a preference.
		IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow) {
			if (!(IsNewRow(pRow) && IsContentEmpty(pRow))) {
				m_dlMacroList->CurSel = pRow;
				OnBtnAddAndSave();
			}
		}
	}
	NxCatchAll("Error in CEmrTextMacroDlg::OnDblClickCellEmrTextMacroList");
}

void CEmrTextMacroDlg::OnSelChangingEmrTextMacroList(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	try {
		// If the new selection is NULL, then undo the change
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
		// If the new selection is valid, we need to do something about the existing selection,
		// and transition the appearance to the new selection
		else {
			if (IDCANCEL == SaveOrDiscardCurrentSelection()) {
				SafeSetCOMPointer(lppNewSel, lpOldSel);
			}
			else {
				HandleListSelectionChange(IRowSettingsPtr(*lppNewSel));
			}
		}
	}
	NxCatchAll("Error in CEmrTextMacroDlg::OnSelChangingEmrTextMacroList");
}

void CEmrTextMacroDlg::OnChangeEditDetailName() 
{
	try {
		if (!m_bIgnoreEditChanges) {
			CString strName;
			GetDlgItemText(IDC_EDIT_DETAIL_NAME, strName);
			m_wndPreview.SetLabelText(strName);
			if (NULL != m_dlMacroList->CurSel) {
				IRowSettingsPtr pRow = m_dlMacroList->CurSel;
				if (IsNewRow(pRow) && strName.IsEmpty() && VarString(pRow->Value[eclTextData]).IsEmpty()) {
					pRow->Value[eclDetailName] = SENTINEL_NEW_NAME;
				} else {
					pRow->Value[eclDetailName] = _bstr_t(strName);
				}
			}
			else {
				// This should never happen
			}
		}
	}
	NxCatchAll("Error in CEmrTextMacroDlg::OnChangeEditDetailName");
}

LRESULT CEmrTextMacroDlg::OnChangePreviewEdit(WPARAM wParam, LPARAM lParam)
{
	try {
		if (!m_bIgnoreEditChanges) {
			BSTR bstrTextData = (BSTR)wParam;
			CString str(bstrTextData);
			SysFreeString(bstrTextData);
			if (NULL != m_dlMacroList->CurSel) {
				IRowSettingsPtr pRow = m_dlMacroList->CurSel;

				// First, update the macro text column
				pRow->Value[eclTextData] = _bstr_t(str);

				// Second, check to see whether the row is a new row with no name. If so, we need to wipe the
				// sentinel text if it's present and there's text content, or restore it if there is no text content
				CString strName;
				GetDlgItemText(IDC_EDIT_DETAIL_NAME, strName);
				if (IsNewRow(pRow) && strName.IsEmpty()) {

					if (str.IsEmpty()) {
						// No text content; ensure the sentinel name is there
						pRow->Value[eclDetailName] = SENTINEL_NEW_NAME;
					}
					else {
						// There is text content; replace the sentinel name with a blank string
						pRow->Value[eclDetailName] = "";
					}

				} else {
					// The row is either not new, or has a detail name; so leave it alone
				}

			} // if (NULL != m_dlMacroList->CurSel) {
		} // if (!m_bIgnoreEditChanges) {
	}
	NxCatchAll("Error in CEmrTextMacroDlg::OnChangePreviewEdit");
	return 0;
}

void CEmrTextMacroDlg::OnBtnAddAndSave() 
{
	try {
		// This will validate and save the currently selected macro, then assign output
		// values for the dialog invoker to use, and dismiss this dialog
		IRowSettingsPtr pCurSel = m_dlMacroList->CurSel;
		if (NULL != pCurSel) {

			// First, validate the content
			if (!ValidateContent(TRUE)) {
				return;
			}

			// Save the content if necessary. If it's a new macro, it will be added
			// to the list. If existing, it will be updated.
			if (IsNewRow(pCurSel) || NeedToSave()) {
				SaveContent();
			}

			// Assign the output values and dismiss this dialog
			m_strResultName = VarString(pCurSel->Value[eclDetailName]);
			m_strResultTextData = VarString(pCurSel->Value[eclTextData]);
			CNxDialog::OnOK();

		} else {
			// This should never happen, but we must consider it nonetheless
			ThrowNxException("OnBtnAddAndSave() was called without a valid row selection!");
		}
	}
	NxCatchAll("Error in CEmrTextMacroDlg::OnBtnAddAndSave");
	
}

void CEmrTextMacroDlg::OnBtnNewMacro() 
{
	try {
		// Save the current selection before creating a new macro
		IRowSettingsPtr pRow = m_dlMacroList->CurSel;
		if (NULL != pRow) {
			BOOL bSaveContent = FALSE;
			if (IsNewRow(pRow)) {
				if (!ValidateContent(TRUE)) {
					return;
				} else {
					bSaveContent = TRUE;
				}
			} else {
				if (NeedToSave()) {
					bSaveContent = TRUE;
				}
			}

			if (bSaveContent) {
				SaveContent();
			}
		}
		// Now make the new macro
		CreateAndSelectEmptyRow();
	}
	NxCatchAll("Error in CEmrTextMacroDlg::OnBtnNewMacro");	
}

void CEmrTextMacroDlg::OnBtnDeleteMacro() 
{
	try {
		m_pRowToDelete = m_dlMacroList->CurSel;
		DeleteMacro();
	}
	NxCatchAll("Error in CEmrTextMacroDlg::OnBtnDeleteMacro");
}

void CEmrTextMacroDlg::OnPopupDeleteMacro()
{
	try {
		DeleteMacro();
	}
	NxCatchAll("Error in CEmrTextMacroDlg::OnPopupDeleteMacro");
}

void CEmrTextMacroDlg::DeleteMacro()
{
	IRowSettingsPtr pRow = m_pRowToDelete;
	m_pRowToDelete = NULL;
	if (NULL != pRow) {
		long nID = VarLong(pRow->Value[eclID]);
		if (-1 != nID) {
			if (IDYES == MessageBox(FormatString("Are you sure you wish to delete the macro '%s'?", VarString(pRow->Value[eclDetailName])), "NexTech Practice", MB_YESNO | MB_ICONQUESTION))
			{
				long nAuditTransactionID = -1;
				try {
					nAuditTransactionID = BeginAuditTransaction();
					ExecuteParamSql("DELETE FROM EMRTextMacroT WHERE ID = {INT}", nID);
					const CString strOldValue = FormatString("'%s' - '%s'", 
						VarString(pRow->Value[eclDetailName]), VarString(pRow->Value[eclTextData]));
					AuditEvent(-1, "", nAuditTransactionID, aeiEMRTextMacroDeleted, nID, strOldValue, "", aepMedium, aetDeleted);
					CommitAuditTransaction(nAuditTransactionID);
				}
				catch (...) {
					if (-1 != nAuditTransactionID) {
						RollbackAuditTransaction(nAuditTransactionID);
					}
					throw;
				}
				m_dlMacroList->RemoveRow(pRow);
				m_dlMacroList->CurSel = m_dlMacroList->GetFirstRow();
				HandleListSelectionChange(m_dlMacroList->CurSel);
			}
			else {
				// The user changed their mind
			}
		}
		else {
			// This should never happen
		}
	}
	else {
		ThrowNxException("Called DeleteMacro() without a valid row selection!");
	}
}

BOOL CEmrTextMacroDlg::PreTranslateMessage(MSG* pMsg) 
{
	// (c.haag 2008-06-16 09:36) - Override tab ordering to span the parent dialog
	const BOOL bIsShiftKeyDown = (GetAsyncKeyState(VK_SHIFT) & 0x80000000);
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_TAB) {
		if (bIsShiftKeyDown && IDC_EMR_TEXT_MACRO_LIST == GetFocus()->GetDlgCtrlID()) {
			CWnd* pWnd = m_wndPreview.GetDlgItem(EMR_TEXT_MACRO_EDIT_IDC);
			if (NULL != pWnd) {
				pWnd->SetFocus();
				return TRUE;
			}
		}
		else if (!bIsShiftKeyDown && IDC_EDIT_DETAIL_NAME == GetFocus()->GetDlgCtrlID()) {
			CWnd* pWnd = m_wndPreview.GetDlgItem(EMR_TEXT_MACRO_EDIT_IDC);
			if (NULL != pWnd) {
				pWnd->SetFocus();
				return TRUE;
			}
		}
	}
	return CNxDialog::PreTranslateMessage(pMsg);
}

void CEmrTextMacroDlg::OnOK()
{
	// Don't let the user dismiss the dialog by pressing <Enter> from the macro name edit box
}

void CEmrTextMacroDlg::OnCancel()
{
	// (c.haag 2008-07-25 14:07) - PLID 26038 - The user may have opened the dialog to edit an
	// existing macro. Warn them if they would lose their changes
	try {
		if (IDCANCEL == SaveOrDiscardCurrentSelection()) {
			return;
		}

		// If there is text in the new macro row, ask the user if they want to save it
		IRowSettingsPtr pRow = m_dlMacroList->FindByColumn(eclID, -1L, NULL, VARIANT_FALSE);
		if (NULL != pRow) {
			if (!IsContentEmpty(pRow)) {
				CString strMacroName = VarString(pRow->Value[eclDetailName]).IsEmpty() ? VarString(pRow->Value[eclTextData]) : VarString(pRow->Value[eclDetailName]);
				if (strMacroName.GetLength() > 100) {
					strMacroName = strMacroName.Left(100) + "...";
				}
				switch (MessageBox(FormatString("Would you like to add the unsaved macro '%s' to your data now?", strMacroName), "NexTech Practice", MB_YESNOCANCEL | MB_ICONQUESTION)) {
				case IDCANCEL:
					return;
				case IDYES:
					m_dlMacroList->CurSel = pRow;
					if (!ValidateContent(TRUE)) {
						return;
					}
					SaveContent();
					break;
				case IDNO:
					break;
				}
			}
		}

		CNxDialog::OnCancel();
	}
	NxCatchAll("Error in CEmrTextMacroDlg::OnCancel");
}

void CEmrTextMacroDlg::OnRButtonUpEmrTextMacroList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	// Let the user delete macros with right-click operations. We don't want to change the list selection
	// because the dialog is designed to save the contents of the selected row before the selection changes.
	// So, just track the row to delete, and invoke a pop-up.
	try {
		IRowSettingsPtr pRow(lpRow);
		if (NULL != pRow && VarLong(pRow->Value[eclID]) > -1) {
			CMenu mPopup;
			CPoint pt;
			GetCursorPos(&pt);
			m_pRowToDelete = pRow;
			mPopup.CreatePopupMenu();
			CString strMacroName = VarString(pRow->Value[eclDetailName]).IsEmpty() ? VarString(pRow->Value[eclTextData]) : VarString(pRow->Value[eclDetailName]);
			if (strMacroName.GetLength() > 20) {
				strMacroName = strMacroName.Left(20) + "...";
			}
			mPopup.InsertMenu(0, MF_BYPOSITION, ID_DELETE_MACRO, FormatString("Delete macro '%s'", strMacroName));
			mPopup.TrackPopupMenu(TPM_LEFTALIGN,pt.x, pt.y,this);
		}
	}
	NxCatchAll("Error in CEmrTextMacroDlg::OnRButtonUpEmrTextMacroList");
}

