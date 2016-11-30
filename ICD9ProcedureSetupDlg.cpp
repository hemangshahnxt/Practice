// ICD9ProcedureSetupDlg.cpp : implementation file
//

// (a.walling 2007-06-20 12:09) - PLID 26412 - Dialog to set up the ICD9 Procedure Codes
// (r.gonet 03/21/2014) - PLID 61240 - Can now store both ICD-9-CM Vol. 3 codes and ICD-10-PCS codes

#include "stdafx.h"
#include "administratorRc.h"
#include "ICD9ProcedureSetupDlg.h"

#include "MultiSelectDlg.h"
#include "DontShowDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CICD9ProcedureSetupDlg dialog


CICD9ProcedureSetupDlg::CICD9ProcedureSetupDlg(CWnd* pParent)
	: CNxDialog(CICD9ProcedureSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CICD9ProcedureSetupDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CICD9ProcedureSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CICD9ProcedureSetupDlg)
	DDX_Control(pDX, IDC_EDIT_ICD9V3_CODE, m_nxeditEditIcd9v3Code);
	DDX_Control(pDX, IDC_EDIT_ICD9V3_DESCRIPTION, m_nxeditEditIcd9v3Description);
	DDX_Control(pDX, IDC_BTN_ICD9V3_ADD, m_btnIcd9v3Add);
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_BTN_ICD9V3_LINK_TO_MULTIPLE, m_btnIcd9v3LinkToMultiple);
	DDX_Control(pDX, IDC_BTN_ICD9V3_LINK_TO_UNLINKED, m_btnIcd9v3LinkToUnlinked);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CICD9ProcedureSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CICD9ProcedureSetupDlg)
	ON_BN_CLICKED(IDC_BTN_ICD9V3_ADD, OnBtnAdd)
	ON_BN_CLICKED(IDC_BTN_ICD9V3_LINK_TO_MULTIPLE, OnBtnLinkToMultiple)
	ON_EN_CHANGE(IDC_EDIT_ICD9V3_CODE, OnChangeEditIcd9v3Code)
	ON_BN_CLICKED(IDC_BTN_ICD9V3_LINK_TO_UNLINKED, OnBtnIcd9v3LinkToUnlinked)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CICD9ProcedureSetupDlg message handlers

BEGIN_EVENTSINK_MAP(CICD9ProcedureSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CICD9ProcedureSetupDlg)
	ON_EVENT(CICD9ProcedureSetupDlg, IDC_LIST_ICD9V3_SETUP, 28 /* CurSelWasSet */, OnCurSelWasSetList, VTS_NONE)
	ON_EVENT(CICD9ProcedureSetupDlg, IDC_LIST_ICD9V3_SETUP, 9 /* EditingFinishing */, OnEditingFinishingList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CICD9ProcedureSetupDlg, IDC_LIST_ICD9V3_SETUP, 10 /* EditingFinished */, OnEditingFinishedList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CICD9ProcedureSetupDlg, IDC_LIST_ICD9V3_SETUP, 6 /* RButtonDown */, OnRButtonDownListIcd9v3Setup, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CICD9ProcedureSetupDlg::OnCurSelWasSetList() 
{
	try {
		if (m_dlList != NULL && m_dlList->GetCurSel() != NULL) {
			EnableDlgItem(IDC_BTN_ICD9V3_LINK_TO_MULTIPLE, TRUE);
			EnableDlgItem(IDC_BTN_ICD9V3_LINK_TO_UNLINKED, TRUE);
		} else {
			EnableDlgItem(IDC_BTN_ICD9V3_LINK_TO_MULTIPLE, FALSE);
			EnableDlgItem(IDC_BTN_ICD9V3_LINK_TO_UNLINKED, FALSE);
		}
	} NxCatchAll("Error in OnCurSelWasSetList()");
}

void CICD9ProcedureSetupDlg::OnEditingFinishingList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, LPCTSTR strUserEntered, VARIANT FAR* pvarNewValue, BOOL FAR* pbCommit, BOOL FAR* pbContinue) 
{
	try {
		if (*pbCommit && nCol == eipCode) {
			long nLength = strlen(strUserEntered);

			CString str(strUserEntered);
			str.TrimLeft();
			str.TrimRight();

			NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
			long nID = VarLong(pRow->GetValue(0));

			if (!VerifyCode(str, nID)) {
				*pbCommit = FALSE;
				*pbContinue = FALSE;
			}
		} else if (*pbCommit && nCol == eipDescription) {
			long nLength = strlen(strUserEntered);

			if (nLength > 254) {
				MessageBox("Description cannot exceed 254 characters", "Description Too Long", MB_ICONERROR|MB_OK);
				*pbCommit = FALSE;
				*pbContinue = FALSE;

				m_dlList->SetEditingHighlight(254, nLength, VARIANT_FALSE);
			}
		}
	} NxCatchAll("Error in CICD9ProcedureSetupDlg::OnEditingFinishingList");
}

void CICD9ProcedureSetupDlg::OnEditingFinishedList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (bCommit && pRow != NULL) {
			long nID = VarLong(pRow->GetValue(eipID));

			if (nCol == eipCode) {
				// (r.gonet 03/21/2014) - PLID 61240 - Parameterized
				ExecuteParamSql("UPDATE ICD9ProcedureT SET Code = {STRING} WHERE ID = {INT}", VarString(varNewValue), nID);
			} else if (nCol == eipDescription) {
				// (r.gonet 03/21/2014) - PLID 61240 - Parameterized
				ExecuteParamSql("UPDATE ICD9ProcedureT SET Description = {STRING} WHERE ID = {INT}", VarString(varNewValue), nID);
			}
		}
	} NxCatchAll("Error in CICD9ProcedureSetupDlg::OnEditingFinishedList");
}

void CICD9ProcedureSetupDlg::OnBtnAdd() 
{
	try {
		CString strCode, strName;
		GetDlgItemText(IDC_EDIT_ICD9V3_CODE, strCode);
		GetDlgItemText(IDC_EDIT_ICD9V3_DESCRIPTION, strName);

		strCode.TrimLeft(); strCode.TrimRight();
		strName.TrimLeft(); strName.TrimRight();

		if (!VerifyCode(strCode))
			return;

		if (strName.GetLength() > 254) {
			MessageBox("Description cannot exceed 254 characters.", "Description Too Long", MB_ICONERROR|MB_OK);
			GetDlgItem(IDC_EDIT_ICD9V3_DESCRIPTION)->SetFocus();
			return;
		}

		if (strName.IsEmpty()) {
			if (IDNO == MessageBox("You should not have an empty description. Are you sure you want to continue?", NULL, MB_ICONEXCLAMATION|MB_YESNO)) {
				return;
			}
		}

		// (r.gonet 03/21/2014) - PLID 61240 - Parameterized
		ADODB::_RecordsetPtr prs = CreateParamRecordset(
			"SET NOCOUNT ON "
			"INSERT INTO ICD9ProcedureT(Code, Description) VALUES({STRING}, {STRING}) "
			"SELECT CONVERT(int, SCOPE_IDENTITY()) AS ID "
			"SET NOCOUNT OFF ", strCode, strName);

		long nID = AdoFldLong(prs, "ID");

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->GetNewRow();

		pRow->PutValue(eipID, _variant_t(nID));
		pRow->PutValue(eipCode, _variant_t(strCode));
		pRow->PutValue(eipDescription, _variant_t(strName));

		m_dlList->AddRowSorted(pRow, NULL);
		m_dlList->PutCurSel(pRow);

		SetDlgItemText(IDC_EDIT_ICD9V3_CODE, "");
		SetDlgItemText(IDC_EDIT_ICD9V3_DESCRIPTION, "");

		// move the focus back to the code for rapid entry.
		GetDlgItem(IDC_EDIT_ICD9V3_CODE)->SetFocus();

	} NxCatchAll("Error in CICD9ProcedureSetupDlg::OnBtnAdd");
}

void CICD9ProcedureSetupDlg::OnBtnLinkToMultiple() 
{
	// (r.gonet 03/21/2014) - PLID 61240 - Changed wording to remove reference to ICD-9.
	DontShowMeAgain(this, "The current ICD Procedure code will be applied to all of the CPT codes you select. Any existing procedure code will be overwritten. Use the 'Link to Unlinked CPT Codes...' button to only see CPT codes which do not already have a procedure code associated.", "ICD9CMProcedureCodeSetupMultiple", "");
	LinkToMultiple();
}

void CICD9ProcedureSetupDlg::LinkToMultiple(BOOL bUnlinkedOnly /*= FALSE*/)
{
	// (a.walling 2007-06-20 12:08) - PLID 26413 - Link this ICD9-CM-v3 code to multiple CPT codes
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_dlList->GetCurSel();

		if (pRow) {
			long nID = VarLong(pRow->GetValue(eipID));

			if (bUnlinkedOnly) {
				// (r.gonet 03/21/2014) - PLID 61240 - Parameterized (though only the function)
				if (!ReturnsRecordsParam("SELECT TOP 1 ID FROM CPTCodeT WHERE ICD9ProcedureCodeID IS NULL")) {
					// (r.gonet 03/21/2014) - PLID 61240 - Changed wording to remove reference to ICD-9.
					MessageBox("All CPT codes have an ICD Procedure code associated!", "", MB_ICONINFORMATION|MB_OK);
					return;
				}
			}

			// (j.armen 2012-06-20 15:23) - PLID 49607 - Provide MultiSelect Sizing ConfigRT Entry
			CMultiSelectDlg dlg(this, "CPTCodeT");

			// first we need to get a list of currently selected items
			// (r.gonet 03/21/2014) - PLID 61240 - Parameterized
			ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT ID FROM CPTCodeT WHERE ICD9ProcedureCodeID = {INT}", nID);

			CArray<long, long> arCurSelections;
			while (!prs->eof) {
				arCurSelections.Add(AdoFldLong(prs, "ID"));

				prs->MoveNext();
			}

			// select them on the dialog
			dlg.PreSelect(arCurSelections);

			CStringArray saFields, saNames;

			saFields.Add("Name");
			saNames.Add	("Description");

			saFields.Add("CASE WHEN ServiceT.Active = 1 THEN 'Yes' ELSE 'No' END AS ActiveStatus");
			saNames.Add("Active");

			dlg.m_strNameColTitle = "Code";

			CString strWhere = bUnlinkedOnly ? "CPTCodeT.ICD9ProcedureCodeID IS NULL" : "";

			if (IDOK == dlg.Open("ServiceT INNER JOIN CPTCodeT ON ServiceT.ID = CPTCodeT.ID", strWhere, "CPTCodeT.ID", "CPTCodeT.Code + ' ' + CPTCodeT.SubCode", "Select CPT codes", 0, 0xFFFFFFFF, &saFields, &saNames)) {
				CWaitCursor cws;
				CArray<long, long> arSelections;

				dlg.FillArrayWithIDs(arSelections);

				CParamSqlBatch sqlBatch;

				// (r.gonet 03/14/2014) - PLID 61240 - Fixed an existing bug where we would unlink all CPT codes except
				// the ones selected in the unlinked dialog (which only shows unlinked codes)
				if(!bUnlinkedOnly) {
					for (int i = 0; i < arCurSelections.GetSize(); i++) {
						BOOL bFound = FALSE;

						for (int j = 0; j < arSelections.GetSize(); j++) {
							if (arCurSelections[i] == arSelections[j]) {
								bFound = TRUE;
								break;
							}
						}

						if (!bFound) {
							// this means an item existed previously that does not now, so it needs to have its link removed.
							// (r.gonet 03/21/2014) - PLID 61240 - Parameterized
							sqlBatch.Add("UPDATE CPTCodeT SET ICD9ProcedureCodeID = NULL WHERE ID = {INT}", arCurSelections[i]);
						}
					}
				} else {
					// (r.gonet 03/14/2014) - PLID 61240 - Only the unlinked codes were shown. The non-presence of an existing linked CPT
					// code does not mean anything in this case.
				}

				for (int i = 0; i < arSelections.GetSize(); i++) {
					BOOL bFound = FALSE;

					for (int j = 0; j < arCurSelections.GetSize(); j++) {
						if (arSelections[i] == arCurSelections[j]) {
							bFound = TRUE;
							break;
						}
					}

					if (!bFound) {
						// this means an item exists in the list that was not previously there, so we need to add it.
						// (r.gonet 03/21/2014) - PLID 61240 - Parameterized
						sqlBatch.Add("UPDATE CPTCodeT SET ICD9ProcedureCodeID = {INT} WHERE ID = {INT}", nID, arSelections[i]);
					}
				}

				if (!sqlBatch.IsEmpty()) {
					sqlBatch.Execute(GetRemoteData());
				}
			}
		}
	} NxCatchAll("Error in CICD9ProcedureSetupDlg::OnBtnLinkToMultiple");
}

BOOL CICD9ProcedureSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	try {
		// (z.manning, 05/01/2008) - PLID 29860 - Set button styles
		m_btnIcd9v3Add.AutoSet(NXB_NEW);
		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnIcd9v3LinkToMultiple.AutoSet(NXB_MODIFY);
		m_btnIcd9v3LinkToUnlinked.AutoSet(NXB_MODIFY);

		// for some reason, i always get an invalid pointer error if I try to use the standard
		// function to bind the datalist here.
		m_dlList = BindNxDataList2Ctrl(IDC_LIST_ICD9V3_SETUP, false);
		m_dlList->Requery();
		
		EnableDlgItem(IDC_BTN_ICD9V3_LINK_TO_MULTIPLE, FALSE);
		EnableDlgItem(IDC_BTN_ICD9V3_LINK_TO_UNLINKED, FALSE);

		GetDlgItem(IDC_EDIT_ICD9V3_CODE)->SetFocus();
	} NxCatchAll("Error initializing CICD9ProcedureSetupDlg");

	return FALSE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CICD9ProcedureSetupDlg::OnOK() 
{
	CDialog::OnOK();
}

void CICD9ProcedureSetupDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CICD9ProcedureSetupDlg::OnChangeEditIcd9v3Code() 
{
	try {
		// (j.jones 2010-01-13 08:49) - PLID 31343 - an ICD-9-CM code can be 12.34 or 12.3,
		// we can no longer force a fixed format, instead just force a max length
		// the content is validated before saving so we'll still enforce the formatting
		//FormatItem(IDC_EDIT_ICD9V3_CODE, "##.##");
		// (r.gonet 03/21/2014) - PLID 61240 - ICD-10-PCS codes are exactly 7 characters in length.
		// We now support storing ICD-10-PCS codes in ICD9ProcedureT, so allow that limit here.
		m_nxeditEditIcd9v3Code.SetLimitText(7);
	} NxCatchAll(__FUNCTION__);
}

void CICD9ProcedureSetupDlg::OnRButtonDownListIcd9v3Setup(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {
			m_dlList->PutCurSel(pRow);
			CMenu mnu;
			mnu.CreatePopupMenu();
			mnu.InsertMenu(0, MF_BYPOSITION, 0x4A4, "Delete");
			CPoint pt;
			GetCursorPos(&pt);
			long n = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, this);

			if (n == 0x4A4) {
				// delete!
				long nID = VarLong(pRow->GetValue(0));

				// (r.gonet 03/21/2014) - PLID 61240 - Parameterized
				ADODB::_RecordsetPtr prs = CreateParamRecordset("SELECT COUNT(*) AS Num FROM CPTCodeT WHERE ICD9ProcedureCodeID = {INT}", nID);

				long count;

				if (prs->eof) {
					count = 0;
				} else {
					count = AdoFldLong(prs, "Num", 0);
				}

				if (count > 0) {
					// (r.gonet 03/21/2014) - PLID 61240 - Changed wording to eliminate reference to ICD-9
					if (IDNO == MessageBox(FormatString("This ICD Procedure code is linked to %li CPT codes! Are you sure you want to delete this code?", count), NULL, MB_YESNO | MB_ICONEXCLAMATION)) {
						return;
					}

					// (r.gonet 03/21/2014) - PLID 61240 - Parameterized
					ExecuteParamSql("UPDATE CPTCodeT SET ICD9ProcedureCodeID = NULL WHERE ICD9ProcedureCodeID = {INT}", nID);
				}

				// (r.gonet 03/21/2014) - PLID 61240 - Parameterized
				ExecuteParamSql("DELETE FROM ICD9ProcedureT WHERE ID = {INT}", nID);

				m_dlList->RemoveRow(pRow);
			}
		}
	} NxCatchAll("Error in CICD9ProcedureSetupDlg::OnRButtonDownListIcd9v3Setup");
}


BOOL CICD9ProcedureSetupDlg::VerifyCode(CString strCode, long nID /* = -1 */)
{
	strCode.TrimLeft();
	strCode.TrimRight();

	// (r.gonet 03/21/2014) - PLID 61240 - We no longer restrict the user to a certain format because we store both ICD-9-CM Vol 3 codes and ICD-10-PCS codes.
	// Do check if the code is empty though
	if(strCode.IsEmpty()) {
		MessageBox("You cannot enter a blank code.", "Blank Code", MB_ICONERROR|MB_OK);
		return FALSE;
	}
	if(strCode.GetLength() > 7) {
		MessageBox("Code cannot exceed 7 characters.", "Code Too Long", MB_ICONERROR|MB_OK);
		return FALSE;
	}

	// (r.gonet 03/21/2014) - PLID 61240 - Parameterized
	if (ReturnsRecordsParam("SELECT ID FROM ICD9ProcedureT WHERE Code LIKE {STRING} {SQL}", strCode, nID == -1 ? CSqlFragment("") : CSqlFragment(" AND ID <> {INT}", nID))) {
		MessageBox("You cannot have duplicate codes.", "Duplicate Codes", MB_ICONERROR|MB_OK);
		GetDlgItem(IDC_EDIT_ICD9V3_CODE)->SetFocus();
		return FALSE;
	}

	return TRUE;
}

void CICD9ProcedureSetupDlg::OnBtnIcd9v3LinkToUnlinked() 
{
	try {
		// (r.gonet 03/21/2014) - PLID 61240 - Changed wording to eliminate reference to ICD-9
		DontShowMeAgain(this, "Only CPT Codes that do not already have an ICD Procedure code associated with them will be available for you to select.", "ICD9CMProcedureCodeSetupUnlinked", "");
		LinkToMultiple(TRUE);
	} NxCatchAll(__FUNCTION__);
}
