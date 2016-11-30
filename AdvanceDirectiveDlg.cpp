// AdvanceDirectiveDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PracticeRc.h"
#include "AdvanceDirectiveDlg.h"
#include "SharedAdvanceDirectiveUtils.h" //(e.lally 2010-02-18) PLID 37438 
#include "InternationalUtils.h"
#include "AuditTrail.h"

// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.walling 2010-01-21 16:43) - PLID 37021 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.walling 2009-06-01 09:30) - PLID 34410 - Advance Directives support

// CAdvanceDirectiveDlg dialog

IMPLEMENT_DYNAMIC(CAdvanceDirectiveDlg, CNxDialog)

CAdvanceDirectiveDlg::CAdvanceDirectiveDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvanceDirectiveDlg::IDD, pParent)
{
	m_nPatientID = -1;
}

CAdvanceDirectiveDlg::~CAdvanceDirectiveDlg()
{
}

void CAdvanceDirectiveDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDCANCEL, m_nxibCancel);
	DDX_Control(pDX, IDC_BTN_ADD_OTHER_CONTACT, m_nxibAddOtherContact);	
	DDX_Control(pDX, IDC_BTN_ADD_NEW_DIRECTIVE, m_nxibAddNew);
	DDX_Control(pDX, IDC_CHECK_ADVANCE_DIRECTIVE, m_nxbReviewed);
	DDX_Control(pDX, IDC_EDIT_ADVANCE_DIRECTIVE, m_nxeditDescription);	
	DDX_Control(pDX, IDC_NXCOLORCTRL_AD, m_nxcolor);
	DDX_Control(pDX, IDC_NXCOLORCTRL_ADLIST, m_nxcolorList);	
	DDX_Control(pDX, IDC_AD_DATE_FROM, m_dtpFrom);	
	DDX_Control(pDX, IDC_AD_DATE_TO, m_dtpTo);	
	DDX_Control(pDX, IDC_LABEL_AD_LAST_REVIEWED, m_nxsLastReviewed);
}


BEGIN_MESSAGE_MAP(CAdvanceDirectiveDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_ADD_OTHER_CONTACT, &CAdvanceDirectiveDlg::OnBnClickedBtnAddOtherContact)
	ON_BN_CLICKED(IDC_CHECK_ADVANCE_DIRECTIVE, &CAdvanceDirectiveDlg::OnBnClickedCheckAdvanceDirective)
	ON_EN_CHANGE(IDC_EDIT_ADVANCE_DIRECTIVE, &CAdvanceDirectiveDlg::OnEnChangeEditAdvanceDirective)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_AD_DATE_FROM, &CAdvanceDirectiveDlg::OnDtnDatetimechangeAdDateFrom)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_AD_DATE_TO, &CAdvanceDirectiveDlg::OnDtnDatetimechangeAdDateTo)
	ON_BN_CLICKED(IDC_BTN_ADD_NEW_DIRECTIVE, &CAdvanceDirectiveDlg::OnBnClickedBtnAddNewDirective)
END_MESSAGE_MAP()


// CAdvanceDirectiveDlg message handlers
BOOL CAdvanceDirectiveDlg::OnInitDialog()
{
	ASSERT(m_nPatientID != -1);

	CNxDialog::OnInitDialog();

	try {
		CString strTitle;
		GetWindowText(strTitle);

		strTitle += " for " + GetExistingPatientName(m_nPatientID);
		SetWindowText(strTitle);

		m_nxcolor.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		m_nxcolorList.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		
		m_nxeditDescription.SetLimitText(255);
		m_nxibCancel.AutoSet(NXB_CLOSE);
		m_nxibAddNew.AutoSet(NXB_NEW);
		m_nxibAddOtherContact.AutoSet(NXB_MODIFY);

		m_listDirectives = BindNxDataList2Ctrl(IDC_LIST_ADVANCE_DIRECTIVES, false);

		m_listContacts = BindNxDataList2Ctrl(IDC_CUSTODIAN_CONTACT_LIST, true);
		
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_listContacts->GetNewRow();

			pRow->PutValue(clcID, g_cvarNull);
			pRow->PutValue(clcFirst, "  <None>");
			m_listContacts->AddRowSorted(pRow, NULL);
		}

		m_listType = BindNxDataList2Ctrl(IDC_LIST_ADVANCE_DIRECTIVE_TYPE, false);
		CString strComboSource;
		for (BYTE i = 1; i < 9; i++) {
			CString strLine;
			CString strCode, strDesc;
			//(e.lally 2010-02-18) PLID 37438 - Use the namepace util
			AdvanceDirective::GetDirectiveTypeInfo(i, &strCode, &strDesc);

			NXDATALIST2Lib::IRowSettingsPtr pRow;
			pRow = m_listType->GetNewRow();
			pRow->PutValue(tlcID, (BYTE)i);
			pRow->PutValue(tlcName, (LPCTSTR)strDesc);
			pRow->PutValue(tlcCode, (LPCTSTR)strCode);
			m_listType->AddRowAtEnd(pRow, NULL);

			strLine.Format("%li;%s;", (long)i, strDesc);
			strComboSource += strLine;
		}
		NXDATALIST2Lib::IColumnSettingsPtr pCol = m_listDirectives->GetColumn(lcTypeID);
		pCol->PutComboSource((LPCTSTR)strComboSource);

		RefreshList();

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_listDirectives->GetFirstRow();
		if (pRow) {
			m_listDirectives->PutCurSel(pRow);
			Load(pRow);
		} else {
			if (CheckCurrentUserPermissions(bioPatient, sptWrite, FALSE, 0, TRUE, TRUE)) {
				OnBnClickedBtnAddNewDirective();
			}
		}

		BOOL bEnabled = CheckCurrentUserPermissions(bioPatient, sptWrite);
		if (!bEnabled) {
			m_listDirectives->PutReadOnly(VARIANT_TRUE);
			m_listContacts->PutReadOnly(VARIANT_TRUE);

			m_nxibAddOtherContact.EnableWindow(FALSE);
			m_nxibAddNew.EnableWindow(FALSE);
			m_nxbReviewed.EnableWindow(FALSE);
			m_nxeditDescription.EnableWindow(FALSE);
			m_dtpFrom.EnableWindow(FALSE);
			m_dtpTo.EnableWindow(FALSE);
		}
	} NxCatchAll("CAdvanceDirectiveDlg::OnInitDialog");

	return TRUE;
}


BOOL CAdvanceDirectiveDlg::NeedToSave()
{
	if (!CheckCurrentUserPermissions(bioPatient, sptWrite, FALSE, 0, TRUE, TRUE))
		return FALSE;

	if (!m_changed)
		return FALSE;

	/*
	long nID = -1; 
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_listDirectives->CurSel;
		if (pRow) {
			nID = VarLong(pRow->GetValue(lcID), -1);
		}
	}

	BYTE nTypeID = 0;
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_listType->CurSel;
		if (pRow) {
			nTypeID = VarByte(pRow->GetValue(tlcID));
		}
	}

	if (nID == -1 && nTypeID == 0) {
		return FALSE;
	}
	*/

	return TRUE;
}

BOOL CAdvanceDirectiveDlg::SaveChanges(UINT* pnResult)
{
	try {		
		m_listContacts->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		
		if (pnResult) {
			*pnResult = IDNO;
		}

		if (!NeedToSave())
			return TRUE;

		long nID = -1; 
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_listDirectives->CurSel;
			if (pRow) {
				nID = VarLong(pRow->GetValue(lcID), -1);
			}
		}

		BYTE nTypeID = 0;
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_listType->CurSel;
			if (pRow) {
				nTypeID = VarByte(pRow->GetValue(tlcID));
			}
		}

		/*
		if (nID == -1 && nTypeID == 0) {
			m_changed = false;
			return TRUE;
		}
		*/

		UINT nResult = MessageBox("Do you want to save your changes?", NULL, MB_YESNOCANCEL|MB_ICONEXCLAMATION);
		if (pnResult) {
			*pnResult = nResult;
		}

		if (nResult == IDCANCEL) {
			return FALSE;
		} else if (nResult == IDNO) {
			m_changed = false;
			return TRUE;
		} else {
			// Save and update the list!

			long nCustodianID = -1;
			CString strSaveCustodianID;
			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_listContacts->CurSel;
				if (pRow) {
					nCustodianID = VarLong(pRow->GetValue(clcID));
				}

				if (nCustodianID == -1) {
					strSaveCustodianID = "NULL";
				} else {
					strSaveCustodianID.Format("%li", nCustodianID);
				}
			}

			CString strDescription;
			m_nxeditDescription.GetWindowText(strDescription);

			CString strLastReviewBy;
			_variant_t varLastReview;

			CString strSaveLastReviewBy;
			CString strSaveLastReview;
			{				
				if (m_nxbReviewed.GetCheck()) {
					strLastReviewBy = GetCurrentUserName();
					varLastReview = _variant_t(COleDateTime::GetCurrentTime(), VT_DATE);

					strSaveLastReviewBy.Format("'%s'", _Q(strLastReviewBy));
					strSaveLastReview = "GetDate()";
				} else {
					if (nID == -1) {
						strSaveLastReviewBy = "''";
						strSaveLastReview = "NULL";

						strLastReviewBy = "";
						_variant_t varLastReview = g_cvarNull;
					} else {
						NXDATALIST2Lib::IRowSettingsPtr pRow = m_listDirectives->CurSel;

						strSaveLastReviewBy = "LastReviewBy";
						strSaveLastReview = "LastReviewDate";

						if (pRow) {
							strLastReviewBy = VarString(pRow->GetValue(lcLastReviewBy), "");
							varLastReview = pRow->GetValue(lcLastReview);
						}
					}
				}
			}

			_variant_t varDateFrom = m_dtpFrom.GetValue();
			_variant_t varDateTo = m_dtpTo.GetValue();
			COleDateTime dtFrom; dtFrom.SetStatus(COleDateTime::invalid);
			COleDateTime dtTo; dtTo.SetStatus(COleDateTime::invalid);
			CString strSaveDateFrom;
			if (varDateFrom.vt == VT_DATE) {
				dtFrom = AsDateNoTime(VarDateTime(varDateFrom));
				strSaveDateFrom.Format("'%s'", _Q(FormatDateTimeForSql(dtFrom)));
			} else {
				strSaveDateFrom = "NULL";
			}
			CString strSaveDateTo;
			if (varDateTo.vt == VT_DATE) {
				dtTo = AsDateNoTime(VarDateTime(varDateTo));
				strSaveDateTo.Format("'%s'", _Q(FormatDateTimeForSql(dtTo)));
			} else {
				strSaveDateTo = "NULL";
			}

			if (dtTo.GetStatus() == COleDateTime::valid && dtFrom.GetStatus() == COleDateTime::valid) {
				if (dtFrom > dtTo) {
					MessageBox("The 'from' date is after the 'to' date. Please correct this before saving.", NULL, MB_ICONEXCLAMATION);
					return FALSE;
				}
			}

			if (nTypeID <= 0) {
				// fail
				MessageBox("A type must be selected.", NULL, MB_ICONEXCLAMATION);
				return FALSE;
			}

			CString strOld, strNew;

			long nExistingID = nID;
			if (nID == -1) {
				// new
				ADODB::_RecordsetPtr prs = CreateRecordset(
					"SET NOCOUNT ON;\r\n"
					"INSERT INTO AdvanceDirectivesT(PatientID, TypeID, Description, DateFrom, DateTo, CustodianID, LastReviewBy, LastReviewDate) "
					"VALUES(%li, %li, '%s', %s, %s, %s, %s, %s)\r\n"
					"SET NOCOUNT OFF;\r\n"
					"SELECT Convert(int, SCOPE_IDENTITY()) AS NewID",
					m_nPatientID, nTypeID, _Q(strDescription), strSaveDateFrom, strSaveDateTo, strSaveCustodianID, strSaveLastReviewBy, strSaveLastReview);

				if (!prs->eof) {
					nID = AdoFldLong(prs, "NewID");
				}
			} else {
				// existing
				strOld = GetAuditString(nID);

				ExecuteSql(					
					"UPDATE AdvanceDirectivesT SET TypeID = %li, Description = '%s', DateFrom = %s, DateTo = %s, CustodianID = %s, "
					"LastReviewBy = %s, LastReviewDate = %s WHERE ID = %li",
					nTypeID, _Q(strDescription), strSaveDateFrom, strSaveDateTo, strSaveCustodianID, 
					strSaveLastReviewBy, strSaveLastReview, nID);
			}

			strNew = GetAuditString(nID);
			if (strOld != strNew) {
				AuditEvent(m_nPatientID, GetExistingPatientName(m_nPatientID), BeginNewAuditEvent(), aeiAdvanceDirectives, nID, strOld, strNew, aepLow, nExistingID == -1 ? aetCreated : aetChanged);
			}

			{
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_listDirectives->CurSel;

				pRow->PutValue(lcID, nID);
				pRow->PutValue(lcTypeID, nTypeID == -1 ? g_cvarNull : nTypeID);
				pRow->PutValue(lcDescription, (LPCTSTR)strDescription);
				pRow->PutValue(lcDateFrom, varDateFrom);
				pRow->PutValue(lcDateTo, varDateTo);
				pRow->PutValue(lcLastReview, varLastReview);
				pRow->PutValue(lcLastReviewBy, (LPCTSTR)strLastReviewBy);
				pRow->PutValue(lcCustodianID, lcCustodianID == -1 ? g_cvarNull : nCustodianID);
			}

			m_changed = false;
			return TRUE;
		}

	} NxCatchAll("CAdvanceDirectiveDlg::SaveChanges");

	return FALSE;
}

CString CAdvanceDirectiveDlg::GetAuditString(long nID)
{
	ADODB::_RecordsetPtr prs = CreateParamRecordset(
		"SELECT AdvanceDirectivesT.*, "
			"CASE WHEN PersonT.ID IS NOT NULL THEN "
				"PersonT.First + ' ' + PersonT.Middle + ' ' + PersonT.Last "
			"ELSE NULL END AS CustodianName "
		"FROM AdvanceDirectivesT "
		"LEFT JOIN PersonT ON PersonT.ID = AdvanceDirectivesT.CustodianID "
		"WHERE AdvanceDirectivesT.ID = {INT}", nID
		);

	CString str;

	if (!prs->eof) {
		BYTE nOldTypeID = AdoFldByte(prs, "TypeID", -1);
		
		CString strOldType;
		if (nOldTypeID != -1) {
			//(e.lally 2010-02-18) PLID 37438 - Use the namespace util
			AdvanceDirective::GetDirectiveTypeInfo(nOldTypeID, NULL, &strOldType);
		}

		CString strOldDescription = AdoFldString(prs, "Description", "");

		COleDateTime dtNull;
		dtNull.SetStatus(COleDateTime::null);

		COleDateTime dtFrom = AdoFldDateTime(prs, "DateFrom", dtNull);
		COleDateTime dtTo = AdoFldDateTime(prs, "DateTo", dtNull);

		CString strOldLastReviewBy = AdoFldString(prs, "LastReviewBy", "");
		
		COleDateTime dtLastReview = AdoFldDateTime(prs, "LastReviewDate", dtNull);

		CString strOldCustodianName = AdoFldString(prs, "CustodianName", "");
		while (strOldCustodianName.Replace("  ", " ")) {};

		str = strOldType;

		if (!strOldDescription.IsEmpty()) {
			str += FormatString(" - %s;", strOldDescription);
		}

		if (dtFrom.GetStatus() == COleDateTime::valid) {
			str += FormatString(" From %s;", FormatDateTimeForInterface(dtFrom));
		}
		if (dtTo.GetStatus() == COleDateTime::valid) {
			str += FormatString(" To %s;", FormatDateTimeForInterface(dtTo));
		}

		if (!strOldCustodianName.IsEmpty()) {
			str += FormatString(" Custodian is %s;", strOldCustodianName);
		}

		if (dtLastReview.GetStatus() == COleDateTime::valid) {
			str += FormatString(" Last reviewed %s", FormatDateTimeForInterface(dtLastReview));
		}
		if (!strOldLastReviewBy.IsEmpty()) {
			str += FormatString(" by %s;", strOldLastReviewBy);
		}
	}

	return str;
}

void CAdvanceDirectiveDlg::RefreshList()
{
	CString strWhere;
	strWhere.Format("AdvanceDirectivesT.PatientID = %li", m_nPatientID);
	m_listDirectives->WhereClause = (LPCTSTR)strWhere;
	m_listDirectives->Requery();
	m_listDirectives->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
}

void CAdvanceDirectiveDlg::OnBnClickedBtnAddNewDirective()
{
	try {
		if (!SaveChanges()) {
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_listDirectives->FindByColumn(lcID, g_cvarNull, NULL, VARIANT_TRUE);
		
		BOOL bAdd = FALSE;
		if (pRow == NULL) {
			pRow = m_listDirectives->GetNewRow();		
			bAdd = TRUE;
		}

		for (short i = 0; i < m_listDirectives->GetColumnCount(); i++) {
			pRow->PutValue(i, g_cvarNull);
		}

		pRow->PutValue(lcDescription, "(New Advance Directive)");

		if (bAdd) {
			m_listDirectives->PutCurSel(m_listDirectives->AddRowAtEnd(pRow, NULL));
		}

		Load(pRow);
	} NxCatchAll("CAdvanceDirectiveDlg::OnBnClickedBtnAddNewDirective");
}

void CAdvanceDirectiveDlg::Load(NXDATALIST2Lib::IRowSettingsPtr pRow)
{
	try {
		m_nxbReviewed.SetCheck(BST_UNCHECKED);

		m_dtpFrom.SetValue(COleDateTime::GetCurrentTime());
		m_dtpTo.SetValue(COleDateTime::GetCurrentTime());

		long nID = pRow == NULL ? -1 : VarLong(pRow->GetValue(lcID), -1);
		if (nID == -1) {
			m_dtpFrom.SetValue(g_cvarNull);
			m_dtpTo.SetValue(g_cvarNull);

			m_nxeditDescription.SetWindowText("");

			m_listType->PutCurSel(NULL);
			m_listContacts->PutCurSel(NULL);

			m_nxsLastReviewed.SetWindowText("");

			m_changed = true;
		} else {
			COleDateTime dtNull;
			dtNull.SetStatus(COleDateTime::null);

			long nCustodianID = VarLong(pRow->GetValue(lcCustodianID), -1);

			BYTE nTypeID = VarByte(pRow->GetValue(lcTypeID), 0);

			CString strDescription = VarString(pRow->GetValue(lcDescription), "");

			CString strLastReviewBy = VarString(pRow->GetValue(lcLastReviewBy), "");
			_variant_t varLastReview = pRow->GetValue(lcLastReview);

			_variant_t varDateFrom = pRow->GetValue(lcDateFrom);
			_variant_t varDateTo = pRow->GetValue(lcDateTo);

			if (nCustodianID == -1) {
				m_listContacts->PutCurSel(NULL);
			} else {
				m_listContacts->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
				m_listContacts->SetSelByColumn(lcID, nCustodianID);
			}

			m_listType->SetSelByColumn(tlcID, nTypeID);

			m_nxeditDescription.SetWindowText(strDescription);
			m_dtpFrom.SetValue(varDateFrom);
			m_dtpTo.SetValue(varDateTo);

			m_nxsLastReviewed.SetWindowText(GetLastReviewString(strLastReviewBy, varLastReview));	

			m_changed = false;
		}

	} NxCatchAll("CAdvanceDirectiveDlg::Load");
}

CString CAdvanceDirectiveDlg::GetLastReviewString(const CString& strLastReviewBy, _variant_t varLastReview)
{
	if (varLastReview.vt != VT_DATE) {
		return "No last review information available.";
	}

	CString strReview;
	COleDateTime dtReview = VarDateTime(varLastReview);

	strReview.Format("Last reviewed on %s by '%s'", FormatDateTimeForInterface(dtReview), strLastReviewBy);

	return strReview;
}

void CAdvanceDirectiveDlg::OnBnClickedBtnAddOtherContact()
{
	try {
		/*
		long nExistingID = -1;
		{
			NXDATALIST2Lib::IRowSettingsPtr pExistingRow = m_listContacts->CurSel;
			if (pExistingRow) {
				nExistingID = VarLong(pExistingRow->GetValue(clcID), -1);
			}
		}
		*/

		CMainFrame* pMainFrame = GetMainFrame();
		if (pMainFrame == NULL) return;
		long nNewID = pMainFrame->AddContact(CMainFrame::dctOther, FALSE);

		if (nNewID != -1) {
			m_listContacts->Requery();

			NXDATALIST2Lib::IRowSettingsPtr pRow = m_listContacts->GetNewRow();

			pRow->PutValue(clcID, g_cvarNull);
			pRow->PutValue(clcFirst, "  <None>");
			m_listContacts->AddRowSorted(pRow, NULL);

			m_listContacts->FindByColumn(clcID, nNewID, NULL, VARIANT_TRUE);
			m_changed = true;
		}
	} NxCatchAll("CAdvanceDirectiveDlg::OnBnClickedBtnAddOtherContact");
}

void CAdvanceDirectiveDlg::OnBnClickedCheckAdvanceDirective()
{
	try {
		m_changed = true;
	} NxCatchAll("CAdvanceDirectiveDlg::OnBnClickedCheckAdvanceDirective");
}

void CAdvanceDirectiveDlg::OnOK()
{

}

void CAdvanceDirectiveDlg::OnCancel()
{
	try {
		if (NeedToSave()) {
			UINT nResult;
			if (!SaveChanges(&nResult)) {
				return;
			}

			if (nResult == IDYES) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_listDirectives->CurSel;

				if (pRow) {
					Load(pRow);
				}

				MessageBox("Please review the saved changes before closing.", NULL, MB_ICONINFORMATION);

				return;
			}
		}
	} NxCatchAll("CAdvanceDirectiveDlg::OnCancel");

	CNxDialog::OnCancel();
}

BEGIN_EVENTSINK_MAP(CAdvanceDirectiveDlg, CNxDialog)
	ON_EVENT(CAdvanceDirectiveDlg, IDC_LIST_ADVANCE_DIRECTIVE_TYPE, 2, CAdvanceDirectiveDlg::SelChangedListAdvanceDirectiveType, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CAdvanceDirectiveDlg, IDC_LIST_ADVANCE_DIRECTIVE_TYPE, 1, CAdvanceDirectiveDlg::SelChangingListAdvanceDirectiveType, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CAdvanceDirectiveDlg, IDC_CUSTODIAN_CONTACT_LIST, 2, CAdvanceDirectiveDlg::SelChangedCustodianContactList, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CAdvanceDirectiveDlg, IDC_CUSTODIAN_CONTACT_LIST, 1, CAdvanceDirectiveDlg::SelChangingCustodianContactList, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CAdvanceDirectiveDlg, IDC_LIST_ADVANCE_DIRECTIVES, 1, CAdvanceDirectiveDlg::SelChangingListAdvanceDirectives, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CAdvanceDirectiveDlg, IDC_LIST_ADVANCE_DIRECTIVES, 2, CAdvanceDirectiveDlg::SelChangedListAdvanceDirectives, VTS_DISPATCH VTS_DISPATCH)
END_EVENTSINK_MAP()

void CAdvanceDirectiveDlg::SelChangingListAdvanceDirectives(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		} else if (!SaveChanges()) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll("CAdvanceDirectiveDlg::SelChangingListAdvanceDirectives");
}

void CAdvanceDirectiveDlg::SelChangedListAdvanceDirectives(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pNewRow(lpNewSel);

		if (pNewRow) {
			if (lpOldSel == NULL || !pNewRow->IsSameRow(lpOldSel)) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_listDirectives->FindByColumn(lcID, g_cvarNull, NULL, VARIANT_FALSE);
				if (pRow) {
					m_listDirectives->RemoveRow(pRow);
				}

				Load(lpNewSel);
			}
		}
	} NxCatchAll("CAdvanceDirectiveDlg::SelChangedListAdvanceDirectives");
}

void CAdvanceDirectiveDlg::SelChangedListAdvanceDirectiveType(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

		if (pRow) {
			if (lpOldSel == NULL || VARIANT_FALSE == pRow->IsSameRow(lpOldSel)) {
				m_changed = true;
			}
		} else {
			m_changed = true;
		}
	} NxCatchAll("CAdvanceDirectiveDlg::SelChangedListAdvanceDirectiveType");
}

void CAdvanceDirectiveDlg::SelChangingListAdvanceDirectiveType(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	} NxCatchAll("CAdvanceDirectiveDlg::SelChangingListAdvanceDirectiveType");
}

void CAdvanceDirectiveDlg::SelChangingCustodianContactList(LPDISPATCH lpOldSel, LPDISPATCH* lppNewSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(*lppNewSel);

		if (pRow) {
			_variant_t varValue = pRow->GetValue(lcID);

			if (varValue.vt != VT_I4) {
				
				if (*lppNewSel) {
					(*lppNewSel)->Release();
				} 
				*lppNewSel = NULL;
			}
		}
	} NxCatchAll("CAdvanceDirectiveDlg::SelChangingCustodianContactList");
}

void CAdvanceDirectiveDlg::SelChangedCustodianContactList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);

		if (pRow) {
			if (lpOldSel == NULL || VARIANT_FALSE == pRow->IsSameRow(lpOldSel)) {
				m_changed = true;
			}
		} else if (lpOldSel != NULL) {
			m_changed = true;
		}
	} NxCatchAll("CAdvanceDirectiveDlg::SelChangedCustodianContactList");
}

void CAdvanceDirectiveDlg::OnEnChangeEditAdvanceDirective()
{
	m_changed = true;
}

void CAdvanceDirectiveDlg::OnDtnDatetimechangeAdDateFrom(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
	
	m_changed = true;

	*pResult = 0;
}

void CAdvanceDirectiveDlg::OnDtnDatetimechangeAdDateTo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
		
	m_changed = true;

	*pResult = 0;
}