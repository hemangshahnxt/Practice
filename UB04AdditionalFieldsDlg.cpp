// CUB04AdditionalFieldsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "UB04AdditionalFieldsDlg.h"
#include "afxdialogex.h"

//(j.camacho 2016-3-3) PLID 68501 - Create new UB04 dialog
// CUB04AdditionalFieldsDlg dialog

namespace {
	enum ConditionsColumn {
		ccCode = 0,
	};

	enum ValuesColumn {
		vcCode = 0,
		vcAmount,
	};

	enum OccurrenceColumn {
		ocCode = 0,
		ocDate,
	};

	enum OccurrenceSpanColumn {
		oscCode = 0,
		oscFrom,
		oscThrough,
	};

	constexpr long ConditionCodeMaxCount()
	{
		// boxes 18-28 have max of 11 rows
		return 28 - 18 + 1;
	}

	constexpr long ValueCodeMaxCount()
	{
		// box (39 + 40 + 41) * abcd have max of 12 rows
		return (41 - 39 + 1) * 4;
	}

	constexpr long OccurrenceCodeMaxCount()
	{
		// box (31 + 32 + 33 + 34) * ab have max of 8 rows
		return (34 - 31 + 1) * 2;
	}

	constexpr long OccurrenceSpanCodeMaxCount()
	{
		// box (35 + 36) * ab have max of 4 rows
		return (36 - 35 + 1) * 2;
	}
}

IMPLEMENT_DYNAMIC(CUB04AdditionalFieldsDlg, CNxDialog)

CUB04AdditionalFieldsDlg::CUB04AdditionalFieldsDlg(CWnd* pParent, UB04::ClaimInfo claimInfo, UBFormType formType, const CString& strUB92Box79)
	: CNxDialog(IDD_UB04_ADD_FIELDS, pParent)
	, m_claimInfo(claimInfo)
	, m_formType(formType)
	, m_strUB92Box79(strUB92Box79)
{
}

CUB04AdditionalFieldsDlg::~CUB04AdditionalFieldsDlg()
{
}

BOOL CUB04AdditionalFieldsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		m_pConditionList = BindNxDataList2Ctrl(IDC_UB04_CONDITIONAL, false);
		m_pValueList = BindNxDataList2Ctrl(IDC_UB04_VALUECODE, false);
		m_pOccurrenceList = BindNxDataList2Ctrl(IDC_UB04_OCCURENCE, false);
		m_pOccurrenceSpanList = BindNxDataList2Ctrl(IDC_UB04_OCCURENCESPAN, false);

		m_btnOK.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (a.walling 2016-03-10 14:51) - PLID 68561 - UB04 Enhancements - all UB handling in CUB04AdditionalFieldsDlg
		if (m_formType == eUB92) {
			SetWindowText("Additional UB92 Claim Fields");

			((CEdit*)SafeGetDlgItem<CWnd>(IDC_UB04_REMARK))->SetLimitText(20);

			SetDlgItemText(IDC_UB04_CONDITION_LABEL, "Condition Codes");
			SetDlgItemText(IDC_UB04_OCCURENCE_LABEL, "Occurrence Codes");
			SetDlgItemText(IDC_UB04_OCCURENCESPAN_LABEL, "Occurrence Span Codes");
			SetDlgItemText(IDC_UB04_VALUECODE_LABEL, "Value Codes");
			SetDlgItemText(IDC_UB04_REMARK_LABEL, "Procedure Method (UB92 Box 79)");
		}

		SecureControls(false);
		LoadUB04ClaimInfo();

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

// (r.goldschmidt 2016-03-03 13:08) - PLID 68511 - UB04 Enhancements - set up datalists
void CUB04AdditionalFieldsDlg::EmptyAllData()
{
	try {
			m_pConditionList->Clear();
		for (int i = 0; i < ConditionCodeMaxCount(); i++) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConditionList->GetNewRow();
				m_pConditionList->AddRowAtEnd(pRow, NULL);
			}

			m_pValueList->Clear();
		for (int i = 0; i < ValueCodeMaxCount(); i++) {
				NXDATALIST2Lib::IRowSettingsPtr pRow = m_pValueList->GetNewRow();
				m_pValueList->AddRowAtEnd(pRow, NULL);
			}

		m_pOccurrenceList->Clear();
		for (int i = 0; i < OccurrenceCodeMaxCount(); i++) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOccurrenceList->GetNewRow();
			m_pOccurrenceList->AddRowAtEnd(pRow, NULL);
			}

		m_pOccurrenceSpanList->Clear();
		for (int i = 0; i < OccurrenceSpanCodeMaxCount(); i++) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOccurrenceSpanList->GetNewRow();
			m_pOccurrenceSpanList->AddRowAtEnd(pRow, NULL);
			}

		SetDlgItemText(IDC_UB04_REMARK, "");

	}NxCatchAll(__FUNCTION__);
		}

// (r.goldschmidt 2016-03-03 13:08) - PLID 68511 - UB04 Enhancements - set up datalists
void CUB04AdditionalFieldsDlg::SecureControls(bool bResetData)
{
	try {
		if (bResetData) {
			EmptyAllData();
		}

		if (m_bReadOnly) {
			m_pConditionList->PutReadOnly(VARIANT_TRUE);
			m_pValueList->PutReadOnly(VARIANT_TRUE);
			m_pOccurrenceList->PutReadOnly(VARIANT_TRUE);
			m_pOccurrenceSpanList->PutReadOnly(VARIANT_TRUE);
			GetDlgItem(IDC_UB04_REMARK)->EnableWindow(FALSE);
		}

	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2016-03-03 17:17) - PLID 68506 - UB04 Enhancements - Load C++ structures into the dialog
void CUB04AdditionalFieldsDlg::LoadUB04ClaimInfo()
{
	try {

		int i = 0;

		m_pConditionList->Clear();		
		for (auto it = m_claimInfo.conditions.begin(); it < m_claimInfo.conditions.end() && i < ConditionCodeMaxCount(); it++) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConditionList->GetNewRow();
			pRow->PutValue(ccCode, _bstr_t(*it));
			m_pConditionList->AddRowAtEnd(pRow, NULL);
			i++;
		}
		for (; i < ConditionCodeMaxCount(); i++) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pConditionList->GetNewRow();
			m_pConditionList->AddRowAtEnd(pRow, NULL);
		}

		i = 0;
		m_pValueList->Clear();
		for (auto it = m_claimInfo.values.begin(); it < m_claimInfo.values.end() && i < ValueCodeMaxCount(); it++) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pValueList->GetNewRow();
			pRow->PutValue(vcCode, _bstr_t((*it).code));
			pRow->PutValue(vcAmount, _variant_t((*it).amount));
			m_pValueList->AddRowAtEnd(pRow, NULL);
			i++;
		}
		for (; i < ValueCodeMaxCount(); i++) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pValueList->GetNewRow();
			m_pValueList->AddRowAtEnd(pRow, NULL);
		}

		i = 0;
		m_pOccurrenceList->Clear();
		for (auto it = m_claimInfo.occurrences.begin(); it < m_claimInfo.occurrences.end() && i < OccurrenceCodeMaxCount(); it++) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOccurrenceList->GetNewRow();
			pRow->PutValue(ocCode, _bstr_t((*it).code));
			pRow->PutValue(ocDate, _variant_t((*it).date, VT_DATE));
			m_pOccurrenceList->AddRowAtEnd(pRow, NULL);
			i++;
		}
		for (; i < OccurrenceCodeMaxCount(); i++) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOccurrenceList->GetNewRow();
			m_pOccurrenceList->AddRowAtEnd(pRow, NULL);
		}

		i = 0;
		m_pOccurrenceSpanList->Clear();
		for (auto it = m_claimInfo.occurrenceSpans.begin(); it < m_claimInfo.occurrenceSpans.end() && i < OccurrenceSpanCodeMaxCount(); it++) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOccurrenceSpanList->GetNewRow();
			pRow->PutValue(oscCode, _bstr_t((*it).code));
			pRow->PutValue(oscFrom, _variant_t((*it).from, VT_DATE));
			pRow->PutValue(oscThrough, _variant_t((*it).to, VT_DATE));
			m_pOccurrenceSpanList->AddRowAtEnd(pRow, NULL);
			i++;
		}
		for (; i < OccurrenceSpanCodeMaxCount(); i++) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pOccurrenceSpanList->GetNewRow();
			m_pOccurrenceSpanList->AddRowAtEnd(pRow, NULL);
		}

		SetDlgItemText(IDC_UB04_REMARK, m_claimInfo.remarks);

		// (a.walling 2016-03-10 14:51) - PLID 68561 - UB04 Enhancements - all UB handling in CUB04AdditionalFieldsDlg
		if (m_formType == eUB92) {
			SetDlgItemText(IDC_UB04_REMARK, m_strUB92Box79);
		}

	}NxCatchAll(__FUNCTION__);
}

void CUB04AdditionalFieldsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
}


BEGIN_MESSAGE_MAP(CUB04AdditionalFieldsDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MESSAGE_MAP()

// CUB04AdditionalFieldsDlg message handlers



void CUB04AdditionalFieldsDlg::OnOK()
{
	try {
		// (r.goldschmidt 2016-03-04 12:28) - PLID 68507 - UB04 Enhancements - Save dialog values to C++ structures
		if (!m_bReadOnly) {
			//for validation
			bool bHasIncompleteData = false;

			//clear bill data and set according to current state of datalists

			//value codes
			m_claimInfo.values.clear();
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pValueList->GetFirstRow();
			while (pRow) {
				_variant_t vCode = pRow->GetValue(vcCode);
				_variant_t vAmount = pRow->GetValue(vcAmount);
				if ((vCode.vt == VT_NULL || vCode.vt == VT_EMPTY) && (vAmount.vt == VT_NULL || vAmount.vt == VT_EMPTY)) {
					// it's all empty/null, do nothing
				}
				else if ((vCode.vt == VT_NULL || vCode.vt == VT_EMPTY) || (vAmount.vt == VT_NULL || vAmount.vt == VT_EMPTY)) {
					// only partially empty/null
					bHasIncompleteData = true;
				}
				else {
					// add the data to the claim info
					m_claimInfo.values.push_back({ VarString(vCode), VarCurrency(vAmount) });
				}
				pRow = pRow->GetNextRow();
			}

			// condition codes
			m_claimInfo.conditions.clear();
			pRow = m_pConditionList->GetFirstRow();
			while (pRow) {
				_variant_t vCode = pRow->GetValue(ccCode);
				if (vCode.vt != VT_NULL && vCode.vt != VT_EMPTY) {
					m_claimInfo.conditions.push_back(VarString(vCode));
				}
				pRow = pRow->GetNextRow();
			}

			// occurence codes
			m_claimInfo.occurrences.clear();
			pRow = m_pOccurrenceList->GetFirstRow();
			while (pRow) {
				_variant_t vCode = pRow->GetValue(ocCode);
				_variant_t vDate = pRow->GetValue(ocDate);
				if ((vCode.vt == VT_NULL || vCode.vt == VT_EMPTY) && (vDate.vt == VT_NULL || vDate.vt == VT_EMPTY)) {
					// it's all empty/null, do nothing
				}
				else if ((vCode.vt == VT_NULL || vCode.vt == VT_EMPTY) || (vDate.vt == VT_NULL || vDate.vt == VT_EMPTY)) {
					// only partially empty/null
					bHasIncompleteData = true;
				}
				else {
					// add the data to the claim info
					m_claimInfo.occurrences.push_back({ VarString(vCode), VarDateTime(vDate) });
				}
				pRow = pRow->GetNextRow();
			}

			// occurence span codes
			m_claimInfo.occurrenceSpans.clear();
			pRow = m_pOccurrenceSpanList->GetFirstRow();
			while (pRow) {
				_variant_t vCode = pRow->GetValue(oscCode);
				_variant_t vFrom = pRow->GetValue(oscFrom);
				_variant_t vThrough = pRow->GetValue(oscThrough);
				if ((vCode.vt == VT_NULL || vCode.vt == VT_EMPTY)  && (vFrom.vt == VT_NULL || vFrom.vt == VT_EMPTY) && (vThrough.vt == VT_NULL || vThrough.vt == VT_EMPTY)) {
					// it's all empty/null, do nothing
				}
				else if ((vCode.vt == VT_NULL || vCode.vt == VT_EMPTY) || (vFrom.vt == VT_NULL || vFrom.vt == VT_EMPTY) || (vThrough.vt == VT_NULL || vThrough.vt == VT_EMPTY)) {
					// only partially empty/null
					bHasIncompleteData = true;
				}
				else {
					m_claimInfo.occurrenceSpans.push_back({ VarString(vCode), VarDateTime(vFrom), VarDateTime(vThrough) });
				}
				pRow = pRow->GetNextRow();
			}

			// (a.walling 2016-03-10 14:51) - PLID 68561 - UB04 Enhancements - all UB handling in CUB04AdditionalFieldsDlg
			if (m_formType == eUB92) {
				// procedure method (UB92Box79)
				GetDlgItemText(IDC_UB04_REMARK, m_strUB92Box79);
			}
			else {
				// remarks
				GetDlgItemText(IDC_UB04_REMARK, m_claimInfo.remarks);
			}

			if (bHasIncompleteData && (IDOK != MessageBox("Some rows do not have complete data. Saving will discard these rows.", NULL, MB_ICONWARNING | MB_OKCANCEL))) {
				return;
			}
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

BEGIN_EVENTSINK_MAP(CUB04AdditionalFieldsDlg, CNxDialog)
	ON_EVENT(CUB04AdditionalFieldsDlg, IDC_UB04_CONDITIONAL, 9, CUB04AdditionalFieldsDlg::EditingFinishingUb04Conditional, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CUB04AdditionalFieldsDlg, IDC_UB04_VALUECODE, 9, CUB04AdditionalFieldsDlg::EditingFinishingUb04Valuecode, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CUB04AdditionalFieldsDlg, IDC_UB04_OCCURENCE, 9, CUB04AdditionalFieldsDlg::EditingFinishingUb04Occurence, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
	ON_EVENT(CUB04AdditionalFieldsDlg, IDC_UB04_OCCURENCESPAN, 9, CUB04AdditionalFieldsDlg::EditingFinishingUb04Occurencespan, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_BSTR VTS_PVARIANT VTS_PBOOL VTS_PBOOL)
END_EVENTSINK_MAP()

// (r.goldschmidt 2016-03-04 13:59) - PLID 68534 - UB04 Enhancements - mild data validation/warnings; allow deleting rows
void CUB04AdditionalFieldsDlg::EditingFinishingUb04Conditional(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		CString strEntered = strUserEntered;

		if (*pbCommit == FALSE)
			return;

		switch (nCol) {
			//validate code
		case ccCode:
			strEntered.Trim(); // trim the entered text if this is the code column
			if (pvarNewValue->vt != VT_BSTR) {
				MessageBox("The entered text is not a valid code.", NULL, MB_ICONWARNING);
				*pbCommit = FALSE;
			}
			else if (strEntered.IsEmpty()) {
				*pvarNewValue = g_cvarNull;
			}
			else {
				if (strEntered.GetLength() != 2) {
					MessageBox("UB04 specifications call for a code made up of 2 alphanumeric characters. The code entered might not be accepted.", NULL, MB_ICONWARNING);
				}
				*pvarNewValue = _variant_t(strEntered).Detach();
			}
			break;
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2016-03-04 13:59) - PLID 68534 - UB04 Enhancements - reject obviously bad data; mild data validation/warnings; allow deleting rows
void CUB04AdditionalFieldsDlg::EditingFinishingUb04Valuecode(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		CString strEntered = strUserEntered;

		if (*pbCommit == FALSE)
			return;		

		switch (nCol) {
			//validate code
		case vcCode:
			strEntered.Trim(); // trim the entered text if this is the code column
			// if deleting, give option to delete everything
			if (strEntered.IsEmpty()) {
				if (varOldValue.vt == VT_EMPTY || varOldValue.vt == VT_NULL) {
					// it was already empty, don't bother with the following warning
				}
				else if (IDYES == MessageBox("Delete all values from this row?", NULL, MB_YESNO)) {
					NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
					pRow->PutValue(vcCode, g_cvarNull);
					pRow->PutValue(vcAmount, g_cvarNull);
					*pbCommit = FALSE;
					return;
				}
				*pvarNewValue = g_cvarEmpty;
			}
			else if (pvarNewValue->vt != VT_BSTR) {
				MessageBox("The entered text is not a valid code.", NULL, MB_ICONWARNING);
				*pbCommit = FALSE;
			}
			else {
				if (strEntered.GetLength() != 2) {
					MessageBox("UB04 specifications call for a code made up of 2 alphanumeric characters. The code entered might not be accepted.", NULL, MB_ICONWARNING);
				}
				*pvarNewValue = _variant_t(strEntered).Detach();
			}
			break;
			//validate amount
		case vcAmount:
			COleCurrency cyAmt = ParseCurrencyFromInterface(strEntered);
			if (strEntered.IsEmpty()) {
				*pvarNewValue = g_cvarEmpty;
			}
			else if (pvarNewValue->vt != VT_CY || cyAmt.GetStatus() != COleCurrency::valid) {
				MessageBox("The entered text is not a valid amount.", NULL, MB_ICONWARNING);
				*pbCommit = FALSE;
			}
			break;
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2016-03-04 13:59) - PLID 68534 - UB04 Enhancements - reject obviously bad data; mild data validation/warnings; allow deleting rows
void CUB04AdditionalFieldsDlg::EditingFinishingUb04Occurence(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		CString strEntered = strUserEntered;

		if (*pbCommit == FALSE)
			return;

		switch (nCol) {
			//validate code
		case ocCode:
			strEntered.Trim(); // trim the entered text if this is the code column
			// if deleting, give option to delete everything
			if (strEntered.IsEmpty()) {
				if (varOldValue.vt == VT_EMPTY || varOldValue.vt == VT_NULL) {
					// it was already empty, don't bother with the following warning
				}
				else if (IDYES == MessageBox("Delete all values from this row?", NULL, MB_YESNO)) {
					NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
					pRow->PutValue(ocCode, g_cvarNull);
					pRow->PutValue(ocDate, g_cvarNull);
					*pbCommit = FALSE;
					return;
				}
				*pvarNewValue = g_cvarEmpty;
			}
			else if (pvarNewValue->vt != VT_BSTR) {
				MessageBox("The entered text is not a valid code.", NULL, MB_ICONWARNING);
				*pbCommit = FALSE;
			}
			else {
				if (strEntered.GetLength() != 2) {
					MessageBox("UB04 specifications call for a code made up of 2 alphanumeric characters. The code entered might not be accepted.", NULL, MB_ICONWARNING);
				}
				*pvarNewValue = _variant_t(strEntered).Detach();
				// (r.goldschmidt 2016-03-08 11:32) - PLID 68497 - UB04 Enhancements - Apply default dates
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				if (pRow->GetValue(ocDate) == g_cvarNull || pRow->GetValue(ocDate) == g_cvarEmpty) {
					pRow->PutValue(ocDate, varDefaultDate);
				}
			}
			break;
			//validate date
		case ocDate:
			if (strEntered.IsEmpty()) {
				*pvarNewValue = g_cvarEmpty;
			}
			//If this isn't a date, is an invalid date, or has been converted to 12/30/1899 and therefore is some crazy thing.
			else if (pvarNewValue->vt != VT_DATE || VarDateTime(*pvarNewValue).GetStatus() != COleDateTime::valid || VarDateTime(*pvarNewValue).GetYear() < 1900) {
				MessageBox("The entered text is not a valid date.", NULL, MB_ICONWARNING);
				*pbCommit = FALSE;
			}
			break;
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2016-03-04 13:59) - PLID 68534 - UB04 Enhancements - reject obviously bad data; mild data validation/warnings; allow deleting rows
void CUB04AdditionalFieldsDlg::EditingFinishingUb04Occurencespan(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, LPCTSTR strUserEntered, VARIANT* pvarNewValue, BOOL* pbCommit, BOOL* pbContinue)
{
	try {
		CString strEntered = strUserEntered;

		if (*pbCommit == FALSE)
			return;

		switch (nCol) {
			//validate code
		case oscCode:
			strEntered.Trim(); // trim the entered text if this is the code column
			// if deleting, give option to delete everything
			if (strEntered.IsEmpty()) {
				if (varOldValue.vt == VT_EMPTY || varOldValue.vt == VT_NULL) {
					// it was already empty, don't bother with the following warning
				}
				else if (IDYES == MessageBox("Delete all values from this row?", NULL, MB_YESNO)) {
					NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
					pRow->PutValue(oscCode, g_cvarNull);
					pRow->PutValue(oscFrom, g_cvarNull);
					pRow->PutValue(oscThrough, g_cvarNull);
					*pbCommit = FALSE;
					return;
				}
				*pvarNewValue = g_cvarEmpty;
			}
			else if (pvarNewValue->vt != VT_BSTR) {
				MessageBox("The entered text is not a valid code.", NULL, MB_ICONWARNING);
				*pbCommit = FALSE;
			}
			else {
				if (strEntered.GetLength() != 2) {
					MessageBox("UB04 specifications call for a code made up of 2 alphanumeric characters. The code entered might not be accepted.", NULL, MB_ICONWARNING);
				}
				*pvarNewValue = _variant_t(strEntered).Detach();
				// (r.goldschmidt 2016-03-08 11:32) - PLID 68497 - UB04 Enhancements - Apply default dates
				NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
				if ((pRow->GetValue(oscFrom) == g_cvarNull || pRow->GetValue(oscFrom) == g_cvarEmpty)
					&& (pRow->GetValue(oscThrough) == g_cvarNull || pRow->GetValue(oscThrough) == g_cvarEmpty)) {
					pRow->PutValue(oscFrom, varDefaultFrom);
					pRow->PutValue(oscThrough, varDefaultThrough);
				}
			}
			break;
			//validate dates
		case oscFrom:
		case oscThrough:
			if (strEntered.IsEmpty()) {
				*pvarNewValue = g_cvarEmpty;
			}
			//If this isn't a date, is an invalid date, or has been converted to 12/30/1899 and therefore is some crazy thing.
			else if (pvarNewValue->vt != VT_DATE || VarDateTime(*pvarNewValue).GetStatus() != COleDateTime::valid || VarDateTime(*pvarNewValue).GetYear() < 1900) {
				MessageBox("The entered text is not a valid date.", NULL, MB_ICONWARNING);
				*pbCommit = FALSE;
			}
			break;
		}
	}NxCatchAll(__FUNCTION__);
}

// (r.goldschmidt 2016-03-08 11:32) - PLID 68497 - UB04 Enhancements - Apply default dates
void CUB04AdditionalFieldsDlg::SetDefaultDates(COleDateTime dtDate, COleDateTime dtFrom, COleDateTime dtThrough)
{
	if (dtDate.GetStatus() != COleDateTime::valid) {
		varDefaultDate = g_cvarNull;
	}
	else {
		varDefaultDate = _variant_t(dtDate, VT_DATE);
	}

	if (dtFrom.GetStatus() != COleDateTime::valid) {
		varDefaultFrom = g_cvarNull;
	}
	else {
		varDefaultFrom = _variant_t(dtFrom, VT_DATE);
	}

	if (dtThrough.GetStatus() != COleDateTime::valid) {
		varDefaultThrough = g_cvarNull;
	}
	else {
		varDefaultThrough = _variant_t(dtThrough, VT_DATE);
	}
}
