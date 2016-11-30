// EmrMedAllergyViewerDlg.cpp : implementation file
// (c.haag 2010-07-28 15:39) - PLID 38928 - Initial implementation. This dialog
// displays medications, allergies, and prescriptions for an EMR.

#include "stdafx.h"
#include "Practice.h"
#include "EmrRc.h"
#include "EmrMedAllergyViewerDlg.h"
#include "EmrEditorDlg.h"
#include "InternationalUtils.h"

// CEmrMedAllergyViewerDlg dialog

// (c.haag 2010-12-13 9:44) - PLID 41817 - Added eRx_ columns
enum EListColumns
{
	eRowID = 0,
	eTextData,
	eExplanation,
	eRx_Refill,
	eRx_Quantity,
	eRx_Unit,
	eForeColor,
	eBackColor,
	eRecordID,
	eDateAssigned,
	eDateDiscontinued,
};

enum EListRows
{
	elrCurrentMedicationsHeader = 1,
	elrCurrentMedication,
	elrAllergiesHeader,
	elrAllergy,
	elrPrescriptionHeader,
	elrPrescription,
};

using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CEmrMedAllergyViewerDlg, CNxDialog)

CEmrMedAllergyViewerDlg::CEmrMedAllergyViewerDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEmrMedAllergyViewerDlg::IDD, pParent)
{
	m_bIncludeInactiveItems = FALSE;
	m_nTopRowID = -1;
	m_nTopRecordID = -1;
}

CEmrMedAllergyViewerDlg::~CEmrMedAllergyViewerDlg()
{
}

// (c.haag 2010-12-13 9:44) - PLID 41817 - Shows or hides a specified column
void CEmrMedAllergyViewerDlg::ShowColumn(short nCol, BOOL bShow)
{
	IColumnSettingsPtr pCol = m_dlItems->GetColumn(nCol);
	if (bShow) {
		pCol->ColumnStyle = csVisible | csWidthData;
		pCol->PutStoredWidth(pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE));
	} else {
		pCol->ColumnStyle = csVisible | csFixedWidth;
		pCol->StoredWidth = 0;
	}
}

// Save the current size and position
void CEmrMedAllergyViewerDlg::SaveSize()
{
	if (!IsIconic()) {
		// get the window rect and save to preferences.
		CRect rRect;
		GetWindowRect(rRect);

		if ( (rRect.Width() > 50) && (rRect.Height() > 50) ) {
			// ensure a reasonable rect
			SetRemotePropertyInt("EmrMedAllergyViewerWidth", rRect.Width(), 0, GetCurrentUserName());
			SetRemotePropertyInt("EmrMedAllergyViewerHeight", rRect.Height(), 0, GetCurrentUserName());
			SetRemotePropertyInt("EmrMedAllergyViewerTop", rRect.top, 0, GetCurrentUserName());
			SetRemotePropertyInt("EmrMedAllergyViewerLeft", rRect.left, 0, GetCurrentUserName());
		} else {
			// Don't save unreasonable values
		}
	}
	SetRemotePropertyInt("EmrMedAllergyViewerIncludeInactive", m_bIncludeInactiveItems, 0, GetCurrentUserName());
}

// Restore the last saved size and position
void CEmrMedAllergyViewerDlg::RestoreSize()
{
	try {
		CRect rRect, rNewRect;

		GetWindowRect(rRect);
		rNewRect = rRect;

		long nWidth = GetRemotePropertyInt("EmrMedAllergyViewerWidth", rRect.Width(), 0, GetCurrentUserName(), true);
		long nHeight = GetRemotePropertyInt("EmrMedAllergyViewerHeight", rRect.Height(), 0, GetCurrentUserName(), true);
		long nTop = GetRemotePropertyInt("EmrMedAllergyViewerTop", GetSystemMetrics(SM_CYFULLSCREEN) / 2 - rRect.Height() / 2, 0, GetCurrentUserName(), true);
		long nLeft = GetRemotePropertyInt("EmrMedAllergyViewerLeft",GetSystemMetrics(SM_CXFULLSCREEN) / 2 - rRect.Width() / 2, 0, GetCurrentUserName(), true);
		
		if ( (nWidth > 50) && (nHeight > 50) ) {
			// ensure a reasonable rect
			rNewRect.left = nLeft;
			rNewRect.top = nTop;
			rNewRect.right = rNewRect.left + nWidth;
			rNewRect.bottom = rNewRect.top + nHeight;

			CRect rDesktopRect;
			GetDesktopWindow()->GetWindowRect(rDesktopRect);
			rDesktopRect.DeflateRect(0, 0, 30, 50); // deflate the rect's bottom right corner to ensure the top left
				// corner of the new window rect is visible and able to move

			// either the top left or top right corner should be in our desktop rect
			CPoint ptTopLeft = rNewRect.TopLeft();
			CPoint ptTopRight = ptTopLeft;
			ptTopRight.x += rNewRect.Width();

			if (! (rDesktopRect.PtInRect(ptTopLeft) || rDesktopRect.PtInRect(ptTopRight) )) {
				rNewRect.CopyRect(rRect); // get the initial rect
				// now set the width and height
				rNewRect.right = rNewRect.left + nWidth;
				rNewRect.bottom = rNewRect.top + nHeight;
			}

			MoveWindow(rNewRect);
		} 
		else {
			// Don't change the size if the bounds are not reasonable
		}
	} NxCatchAll(__FUNCTION__);
}

// Requery the visible list
void CEmrMedAllergyViewerDlg::Requery()
{
	// We need to try to retain our current scroll position, if any
	m_nTopRowID = -1;
	m_nTopRecordID = -1;
	IRowSettingsPtr pTopRow = m_dlItems->TopRow;
	if (NULL != pTopRow && -2 != VarLong(pTopRow->GetValue(eRowID))) {	
		m_nTopRowID = VarLong(pTopRow->GetValue(eRowID));
		m_nTopRecordID = VarLong(pTopRow->GetValue(eRecordID),-1);
	}

	// Build the query
	long nPatientID = GetPatientID();
	COLORREF clrWhite = RGB(255,255,255);
	COLORREF clrGray = RGB(128,128,128);
	COLORREF clrRed = RGB(192,0,0);

	CString strPersonDataFilter = (m_bIncludeInactiveItems) ? FormatString("WHERE PersonID = %d", nPatientID) : FormatString("WHERE PersonID = %d AND Discontinued = 0", nPatientID);
	CString strPatientDataFilter = (m_bIncludeInactiveItems) ? FormatString("WHERE PatientID = %d", nPatientID) : FormatString("WHERE PatientID = %d AND Discontinued = 0", nPatientID);

	// Columns: Row, TextData, RecordID, DateAssigned, Discontinued, ForeColor, BackColor, HasItems
	// (c.haag 2010-08-06 10:40) - PLID 40021 - Sort prescriptions by Rx date descending
	// (j.jones 2011-05-03 10:29) - PLID 43527 - supported Current Medications Sig
	CString strSql = FormatString(
		"/* Current Medications Header */ "
		"(SELECT 1 AS [Row], 'Current Medications' AS TextData, 'Sig' AS Explanation, NULL AS RecordID, NULL AS DateAssigned, 0 AS Discontinued, NULL AS DiscontinuedDate, %d AS ForeColor, 0 AS BackColor, "
		"	CASE WHEN %d IN (SELECT PatientID FROM CurrentPatientMedsT %s) THEN 1 ELSE 0 END AS HasItems "
		"UNION /* Current Medications */ "
		"SELECT 2, Data, Convert(nvarchar(4000), CurrentPatientMedsQ.Sig), "
		"CurrentPatientMedsQ.ID, NULL, Discontinued, DiscontinuedDate, CASE WHEN Discontinued = 0 THEN 0 ELSE %d END, %d, 1 "
		"FROM (SELECT * FROM CurrentPatientMedsT %s) CurrentPatientMedsQ "
		"INNER JOIN DrugList on DrugList.ID = CurrentPatientMedsQ.MedicationID "
		"INNER JOIN EmrDataT ON EmrDataT.ID = DrugList.EmrDataID "

		"UNION /* Allergies Header */ "
		"SELECT 3, 'Allergies', '', NULL, NULL, 0, NULL, %d, %d, "
		"	CASE WHEN %d IN (SELECT PersonID FROM PatientAllergyT %s) THEN 1 ELSE 0 END  "
		"UNION /* Allergies */ "
		"SELECT 4, Data, '', PatientAllergyQ.ID, EnteredDate, Discontinued, DiscontinuedDate, CASE WHEN Discontinued = 0 THEN %d ELSE %d END, %d, 1 "
		"FROM (SELECT * FROM PatientAllergyT %s) PatientAllergyQ "
		"INNER JOIN AllergyT ON PatientAllergyQ.AllergyID = AllergyT.ID "
		"INNER JOIN EmrDataT ON AllergyT.EmrDataID = EmrDataT.ID "

		"UNION /* Prescriptions Header */ \r\n"
		"SELECT 5, 'Prescriptions', 'Explanation', NULL, NULL, 0, NULL, %d, 0, "
		"	CASE WHEN %d IN (SELECT PatientID FROM PatientMedications %s AND Deleted = 0) THEN 1 ELSE 0 END  \r\n"

		") SubQ"
		/* Current Medications Header */
		,clrWhite
		,nPatientID
		,strPatientDataFilter
		/* Current Medications */
		,clrGray
		,clrWhite
		,strPatientDataFilter
		/* Allergies Header */
		,clrWhite
		,clrRed
		,nPatientID
		,strPersonDataFilter
		/* Allergies */
		,clrRed
		,clrGray
		,clrWhite
		,strPersonDataFilter
		/* Prescriptions Header */
		,clrWhite
		,nPatientID
		,strPatientDataFilter
		);

	m_dlItems->FromClause = _bstr_t(strSql);
	if (m_bIncludeInactiveItems) {
		m_dlItems->WhereClause = "HasItems = 1";
	} else {
		m_dlItems->WhereClause = "HasItems = 1 AND Discontinued = 0";
	}
	m_dlItems->Requery();
}

// Update the button text
void CEmrMedAllergyViewerDlg::UpdateButtonText()
{
	if (m_bIncludeInactiveItems) {
		SetDlgItemText(IDC_BTN_TOGGLE_INACTIVE_MEDALLERGIES, "Hide Discontinued Items");
	} else {
		SetDlgItemText(IDC_BTN_TOGGLE_INACTIVE_MEDALLERGIES, "Include Discontinued Items");
	}
}

void CEmrMedAllergyViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CEmrMedAllergyViewerDlg, CNxDialog)
	ON_WM_DESTROY()
	ON_COMMAND(IDOK, OnOK)
	ON_COMMAND(IDCANCEL, OnCancel)
	ON_COMMAND(IDC_BTN_TOGGLE_INACTIVE_MEDALLERGIES, OnToggleInactiveMedAllergies)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CEmrMedAllergyViewerDlg message handlers

BOOL CEmrMedAllergyViewerDlg::OnInitDialog() 
{
	try 
	{
		CNxDialog::OnInitDialog();

		// Bind controls
		m_dlItems = BindNxDataList2Ctrl(IDC_LIST_MEDALLERGYITEMS, false);

		// Bulk cache
		// (c.haag 2010-12-13 10:04) - PLID 41817 - Added EmrMedAllergyViewerColumns
		g_propManager.CachePropertiesInBulk(
			FormatString("CEmrMedAllergyViewerDlg"), 
			propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'EmrMedAllergyViewerWidth' OR "
			"Name = 'EmrMedAllergyViewerHeight' OR "
			"Name = 'EmrMedAllergyViewerTop' OR "
			"Name = 'EmrMedAllergyViewerLeft' OR "
			"Name = 'EmrMedAllergyViewerIncludeInactive' OR "
			"Name = 'EmrMedAllergyViewerColumns' "
			")", 
			_Q(GetCurrentUserName())
		);

		// Set title text so the user can see which patient this is
		SetWindowText(GetPatientName() + " - Saved Current Medication / Allergy / Rx List");

		// Update the Include Inactive Items button text
		m_bIncludeInactiveItems = GetRemotePropertyInt("EmrMedAllergyViewerIncludeInactive", FALSE, 0, GetCurrentUserName(), true);
		UpdateButtonText();
	}
	NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CEmrMedAllergyViewerDlg::OnOK()
{
	// Make it so this window is hidden if an OK action occurs
	ShowWindow(SW_HIDE);
}

void CEmrMedAllergyViewerDlg::OnCancel()
{
	// Make it so this window is hidden if a cancel action occurs
	ShowWindow(SW_HIDE);
}

void CEmrMedAllergyViewerDlg::OnDestroy()
{
	try {
		// Save the size and position
		SaveSize();
	}
	NxCatchAll(__FUNCTION__);

	CNxDialog::OnDestroy();
}

void CEmrMedAllergyViewerDlg::OnToggleInactiveMedAllergies()
{
	try {
		// Toggle the inactive patient items
		m_bIncludeInactiveItems = (m_bIncludeInactiveItems) ? FALSE : TRUE;
		UpdateButtonText();
		Requery();
	}
	NxCatchAll(__FUNCTION__);
}

LRESULT CEmrMedAllergyViewerDlg::OnTableChanged(WPARAM wParam, LPARAM lParam)
{
	try {
		// Handle patient medication and allergy table checkers. There are no table checkers
		// for prescriptions at the time of this implementation.
		switch (wParam) {
			case NetUtils::CurrentPatientMedsT:
			case NetUtils::PatientAllergyT:
				if (GetPatientID() == lParam) {
					Requery();
				}
				break;
		}
	}
	NxCatchAll(__FUNCTION__);
	return 0;
}

BEGIN_EVENTSINK_MAP(CEmrMedAllergyViewerDlg, CNxDialog)
ON_EVENT(CEmrMedAllergyViewerDlg, IDC_LIST_MEDALLERGYITEMS, 18, CEmrMedAllergyViewerDlg::RequeryFinishedListMedallergyitems, VTS_I2)
END_EVENTSINK_MAP()

void CEmrMedAllergyViewerDlg::RequeryFinishedListMedallergyitems(short nFlags)
{
	try {
		// (c.haag 2010-08-09 16:40) - PLID 40021 - We have to add prescriptions here because it's not possible to sort
		// them in the main datalist query.
		long nPatientID = GetPatientID();
		COLORREF clrWhite = RGB(255,255,255);
		COLORREF clrGray = RGB(128,128,128);

		// (c.haag 2010-12-13 9:44) - PLID 41817 - Users can now display prescription details. If they choose not to, we
		// should hide the header columns.
		// (j.jones 2011-06-09 17:13) - PLID 43527 - need to show the Sig column if we have current meds
		IRowSettingsPtr pPrescriptionsHeaderRow = m_dlItems->FindByColumn(eRowID, (long)elrPrescriptionHeader, NULL, VARIANT_FALSE);
		IRowSettingsPtr pCurrentMedsHeaderRow = m_dlItems->FindByColumn(eRowID, (long)elrCurrentMedicationsHeader, NULL, VARIANT_FALSE);
		long nExtraColumnsMask;
		if(pPrescriptionsHeaderRow != NULL || pCurrentMedsHeaderRow != NULL) {
			nExtraColumnsMask = GetRemotePropertyInt("EmrMedAllergyViewerColumns", 0xFFFFFFFF, 0, GetCurrentUserName());
		} else {
			// No prescriptions? We don't need the columns.
			nExtraColumnsMask = 0;
		}
		CString strExtraColumns;
		if (nExtraColumnsMask & 0x00000001) {
			strExtraColumns += ", PatientExplanation";
			if(pCurrentMedsHeaderRow) {
				pCurrentMedsHeaderRow->Value[eExplanation] = "Sig";
			}
			if(pPrescriptionsHeaderRow) {
				pPrescriptionsHeaderRow->Value[eExplanation] = "Explanation";
			}
		}
		// (j.jones 2011-06-09 17:14) - PLID 43257 - the remaining columns don't need to be shown
		// unless we have prescriptions
		if(pPrescriptionsHeaderRow != NULL) {
			if (nExtraColumnsMask & 0x00000002) {
				strExtraColumns += ", RefillsAllowed";
				pPrescriptionsHeaderRow->Value[eRx_Refill] = "Refills";
			}
			if (nExtraColumnsMask & 0x00000004) {
				strExtraColumns += ", Quantity";
				pPrescriptionsHeaderRow->Value[eRx_Quantity] = "Quantity";
			}
			if (nExtraColumnsMask & 0x00000008) {
				strExtraColumns += ", Unit";
				pPrescriptionsHeaderRow->Value[eRx_Unit] = "Units";
			}
		}
		m_dlItems->HeadersVisible = (nExtraColumnsMask != 0) ? VARIANT_TRUE : VARIANT_FALSE;

		CString strPatientDataFilter = (m_bIncludeInactiveItems) ? FormatString("WHERE PatientID = %d", nPatientID) : FormatString("WHERE PatientID = %d AND Discontinued = 0", nPatientID);
		CString strSql = FormatString(
			"SELECT 6 AS Row, Data AS TextData, PatientMedicationsQ.ID AS RecordID, PrescriptionDate AS DateAssigned, Discontinued, DiscontinuedDate, "
			"CASE WHEN Discontinued = 0 THEN 0 ELSE %d END AS ForeColor, %d AS BackColor \r\n"
			"%s "
			"FROM (SELECT * FROM PatientMedications %s AND Deleted = 0) PatientMedicationsQ  \r\n"
			"INNER JOIN DrugList on DrugList.ID = PatientMedicationsQ.MedicationID  \r\n"
			"INNER JOIN EmrDataT ON EmrDataT.ID = DrugList.EmrDataID  \r\n"
			"ORDER BY PrescriptionDate DESC, Data "
			/* Prescriptions */
			,clrGray
			,clrWhite
			,strExtraColumns
			,strPatientDataFilter
			);

		ADODB::_RecordsetPtr prs = CreateRecordsetStd(strSql);
		ADODB::FieldsPtr f = prs->Fields;
		while (!prs->eof)
		{
			IRowSettingsPtr pRow = m_dlItems->GetNewRow();
			pRow->Value[eRowID] = f->Item["Row"]->Value;
			pRow->Value[eTextData] = f->Item["TextData"]->Value;
			if (nExtraColumnsMask & 0x00000001) {
				pRow->Value[eExplanation] = f->Item["PatientExplanation"]->Value;
			}
			if (nExtraColumnsMask & 0x00000002) {
				pRow->Value[eRx_Refill] = f->Item["RefillsAllowed"]->Value;
			}
			if (nExtraColumnsMask & 0x00000004) {
				pRow->Value[eRx_Quantity] = f->Item["Quantity"]->Value;
			}
			if (nExtraColumnsMask & 0x00000008) {
				pRow->Value[eRx_Unit] = f->Item["Unit"]->Value;
			}
			pRow->Value[eForeColor] = f->Item["ForeColor"]->Value;
			pRow->Value[eBackColor] = f->Item["BackColor"]->Value;
			pRow->Value[eRecordID] = f->Item["RecordID"]->Value;
			pRow->Value[eDateAssigned] = f->Item["DateAssigned"]->Value;
			pRow->Value[eDateDiscontinued] = f->Item["DiscontinuedDate"]->Value;
			// According to http://support.microsoft.com/kb/131101 , this is how you convert a COLORREF 
			// (which we store in data) to an OLE COLOR
			pRow->ForeColor = (OLE_COLOR)AdoFldLong(f, "ForeColor");
			pRow->BackColor = (OLE_COLOR)AdoFldLong(f, "BackColor");
			m_dlItems->AddRowAtEnd(pRow, NULL);
			prs->MoveNext();
		}

		// If the list is empty, add "<None>" to it
		if (m_dlItems->GetRowCount() == 0) {
			IRowSettingsPtr pNewRow = m_dlItems->GetNewRow();
			pNewRow->Value[eRowID] = -2L;
			pNewRow->Value[eTextData] = "<None>";
			m_dlItems->AddRowAtEnd(pNewRow, NULL);
			m_nTopRowID = -1;
			m_nTopRecordID = -1;
			return;
		}

		// Restore the current topmost row to keep the vertical scrollbar as consistent as possible with its pre-requery position
		if (-1 != m_nTopRowID) 
		{
			IRowSettingsPtr pCurrentTopRow = m_dlItems->TopRow;
			IRowSettingsPtr pDesiredTopRow = m_dlItems->GetFirstRow();
			while (NULL != pDesiredTopRow)
			{
				if (VarLong(pDesiredTopRow->GetValue(eRowID)) == m_nTopRowID &&
					VarLong(pDesiredTopRow->GetValue(eRecordID),-1) == m_nTopRecordID)
				{
					break;
				}
				else {
					pDesiredTopRow = pDesiredTopRow->GetNextRow();
				}
			}

			// Only set the top row if:					
			if (NULL != pDesiredTopRow  // A. We found the top row in the list from before
				&& pCurrentTopRow == m_dlItems->GetFirstRow() // B. The current top row is the first row in the list. If not, it means the user scrolled it during the requery.				
				) 
			{
				m_dlItems->TopRow = pDesiredTopRow;
			}
			// Discard the ID; we don't need it anymore and we shouldn't leave it hanging around.
			m_nTopRowID = -1;
			m_nTopRecordID = -1;
		}

		// Customize formatting for each row
		IRowSettingsPtr pRow = m_dlItems->GetFirstRow();
		while (NULL != pRow) {

			// We have to show the prescribed date for prescriptions
			if (VarLong(pRow->GetValue(eRowID)) == elrPrescription && // Prescription row
				pRow->GetValue(eDateAssigned).vt == VT_DATE) { // With valid rx date
				CString strNew;
				strNew.Format("%s: %s", FormatDateTimeForInterface(VarDateTime(pRow->GetValue(eDateAssigned)), NULL, dtoDate), VarString(pRow->GetValue(eTextData)));
				pRow->PutValue(eTextData, _bstr_t(strNew));
			}

			// (c.haag 2010-08-06 13:39) - PLID 40021 - Put the discontinued date at the end of each row
			if (pRow->GetValue(eDateDiscontinued).vt == VT_DATE)
			{
				CString strNew;
				strNew.Format("%s - D/C Date %s", VarString(pRow->GetValue(eTextData)), FormatDateTimeForInterface(VarDateTime(pRow->GetValue(eDateDiscontinued)), NULL, dtoDate));
				pRow->PutValue(eTextData, _bstr_t(strNew));
			}

			pRow = pRow->GetNextRow();
		}

		// (c.haag 2010-12-13 9:44) - PLID 41817 - Resize the columns to fit the data.
		if (nExtraColumnsMask != 0) {
			ShowColumn(eTextData, TRUE);
		}
		ShowColumn(eExplanation, nExtraColumnsMask & 0x00000001);
		ShowColumn(eRx_Refill, nExtraColumnsMask & 0x00000002);
		ShowColumn(eRx_Quantity, nExtraColumnsMask & 0x00000004);
		ShowColumn(eRx_Unit, nExtraColumnsMask & 0x00000008);
	}
	NxCatchAll(__FUNCTION__);
}

void CEmrMedAllergyViewerDlg::OnPaint()
{
	// There's an issue with artifacting when resizing this dialog. The edges of the dialog
	// controls tend to stay in place, causing multiple dark lines to appear as you resize it.
	// I did not find a window style that could prevent this, so this code is in place to 
	// remedy the issue.
	CPaintDC dc(this);
	CRect rThis;
	GetClientRect(rThis);
	dc.FillSolidRect(rThis, GetSysColor(COLOR_WINDOW));
}
