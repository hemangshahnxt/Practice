// MarkupFormulaEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include "InventoryRc.h"
#include "MarkupFormulaEditDlg.h"
#include "NxExpression.h"


// CMarkupFormulaEditDlg dialog
// (z.manning 2010-09-15 12:10) - PLID 40319 - Created

using namespace NXDATALIST2Lib;

IMPLEMENT_DYNAMIC(CMarkupFormulaEditDlg, CNxDialog)

CMarkupFormulaEditDlg::CMarkupFormulaEditDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CMarkupFormulaEditDlg::IDD, pParent)
{
}

CMarkupFormulaEditDlg::~CMarkupFormulaEditDlg()
{
}

void CMarkupFormulaEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MARKUP_BACKGROUND, m_nxcolor);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_NEW_MARKUP, m_btnNew);
	DDX_Control(pDX, IDC_DELETE_MARKUP, m_btnDelete);
}


BEGIN_MESSAGE_MAP(CMarkupFormulaEditDlg, CNxDialog)
	ON_BN_CLICKED(IDC_NEW_MARKUP, &CMarkupFormulaEditDlg::OnBnClickedNewMarkup)
	ON_BN_CLICKED(IDC_DELETE_MARKUP, &CMarkupFormulaEditDlg::OnBnClickedDeleteMarkup)
	ON_BN_CLICKED(IDC_MARKUP_HELP, &CMarkupFormulaEditDlg::OnBnClickedMarkupHelp)
	ON_BN_CLICKED(IDC_ROUNDUP_HELP, &CMarkupFormulaEditDlg::OnBnClickedRoundupHelp)
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CMarkupFormulaEditDlg, CNxDialog)
ON_EVENT(CMarkupFormulaEditDlg, IDC_MARKUP_LIST, 18, CMarkupFormulaEditDlg::RequeryFinishedMarkupList, VTS_I2)
ON_EVENT(CMarkupFormulaEditDlg, IDC_MARKUP_LIST, 16, CMarkupFormulaEditDlg::SelChosenMarkupList, VTS_DISPATCH)
END_EVENTSINK_MAP()


// CMarkupFormulaEditDlg message handlers

BOOL CMarkupFormulaEditDlg::OnInitDialog()
{
	try
	{
		CNxDialog::OnInitDialog();

		m_pdlMarkupCombo = BindNxDataList2Ctrl(IDC_MARKUP_LIST, true);

		((CEdit*)GetDlgItem(IDC_MARKUP_NAME))->SetLimitText(50);
		((CEdit*)GetDlgItem(IDC_MARKUP_FORMULA))->SetLimitText(255 - CString(MARKUP_COST_OPERAND).GetLength());

		// (b.eyers 2016-03-14) - PLID 68590 - add a dropdown for round up rules
		m_pRoundUpRule = BindNxDataList2Ctrl(IDC_ROUND_UP_RULE, false);
		NXDATALIST2Lib::IRowSettingsPtr pRow;
		pRow = m_pRoundUpRule->GetNewRow();
		pRow->PutValue(rrcID, NoRoundUpRule);
		pRow->PutValue(rrcName, "Do not round Sales Price");
		m_pRoundUpRule->AddRowAtEnd(pRow, NULL);
		pRow = m_pRoundUpRule->GetNewRow();
		pRow->PutValue(rrcID, RoundUpNearestDollar);
		pRow->PutValue(rrcName, "Round Sales Price up to nearest dollar");
		m_pRoundUpRule->AddRowAtEnd(pRow, NULL);
		pRow = m_pRoundUpRule->GetNewRow();
		pRow->PutValue(rrcID, RoundUpToNine);
		pRow->PutValue(rrcName, "Round Sales Price up to next 9");
		m_pRoundUpRule->AddRowAtEnd(pRow, NULL);

		m_nxcolor.SetColor(GetNxColor(GNC_INVENTORY, 0));
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnNew.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);

	}NxCatchAll(__FUNCTION__);
	return TRUE;
}

void CMarkupFormulaEditDlg::RequeryFinishedMarkupList(short nFlags)
{
	try
	{
		m_pdlMarkupCombo->PutCurSel(m_pdlMarkupCombo->GetFirstRow());
		Load();

	}NxCatchAll(__FUNCTION__);
}

void CMarkupFormulaEditDlg::Load()
{
	IRowSettingsPtr pRow = m_pdlMarkupCombo->GetCurSel();
	if(pRow == NULL)
	{
		// (z.manning 2010-09-15 12:16) - Probably aren't any markups yet, just disable everything
		GetDlgItem(IDC_MARKUP_NAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_MARKUP_FORMULA)->EnableWindow(FALSE);
		GetDlgItem(IDC_DELETE_MARKUP)->EnableWindow(FALSE);
		SetDlgItemText(IDC_MARKUP_NAME, "");
		SetDlgItemText(IDC_MARKUP_FORMULA, "");
		// (b.eyers 2016-03-14) - PLID 68590 - add a dropdown for round up rules, default to no round up rule
		m_pRoundUpRule->SetSelByColumn(rrcID, NoRoundUpRule); 
	}
	else
	{
		GetDlgItem(IDC_MARKUP_NAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_MARKUP_FORMULA)->EnableWindow(TRUE);
		GetDlgItem(IDC_DELETE_MARKUP)->EnableWindow(TRUE);

		SetDlgItemText(IDC_MARKUP_NAME, VarString(pRow->GetValue(mccName),""));

		// (z.manning 2010-09-15 12:25) - The operand is implied in the interface so remove it before
		// putting the formula in the edit control.
		CString strFormula = VarString(pRow->GetValue(mccFormula),"");
		CString strCostOperand = MARKUP_COST_OPERAND;
		if(!IsNew(pRow) && strFormula.Left(strCostOperand.GetLength()) != strCostOperand) {
			ThrowNxException("Invalid markup formula: " + strFormula);
		}
		strFormula.Delete(0, strCostOperand.GetLength());
		SetDlgItemText(IDC_MARKUP_FORMULA, strFormula);

		// (b.eyers 2016-03-14) - PLID 68590 - add a dropdown for round up rules
		long nRoundUpRule = VarLong(pRow->GetValue(mccRoundUpRule), 0);
		m_pRoundUpRule->SetSelByColumn(rrcID, nRoundUpRule);

	}
	m_lpLastChosenRow = pRow;
}

void CMarkupFormulaEditDlg::SelChosenMarkupList(LPDISPATCH lpRow)
{
	try
	{
		IRowSettingsPtr pPreviousRow(m_lpLastChosenRow);

		// (b.eyers 2016-03-14) - PLID 68590 - the check on round up has been moved to the function that is already checking the other pieces
		if(pPreviousRow != NULL && HasRowChanged(pPreviousRow)) {
			CString strPreviousMarkup = VarString(pPreviousRow->GetValue(mccName), "");
			int nResult = MessageBox(FormatString("Would you like to save changes to the '%s' markup?", strPreviousMarkup), NULL, MB_YESNOCANCEL|MB_ICONQUESTION);
			if(nResult == IDNO) {
				Load();
				if(IsNew(pPreviousRow)) {
					m_pdlMarkupCombo->RemoveRow(pPreviousRow);
				}
				return;
			}
			else if(nResult == IDCANCEL) {
				m_pdlMarkupCombo->PutCurSel(pPreviousRow);
				return;
			}
		}
		else {
			Load();
			if(IsNew(pPreviousRow)) {
				m_pdlMarkupCombo->RemoveRow(pPreviousRow);
			}
			return;
		}

		if(ValidateAndSave(pPreviousRow)) {
			Load();
		}
		else {
			m_pdlMarkupCombo->PutCurSel(pPreviousRow);
		}

	}NxCatchAll(__FUNCTION__);
}

BOOL CMarkupFormulaEditDlg::ValidateAndSave(LPDISPATCH lpRowToSave)
{
	IRowSettingsPtr pRow(lpRowToSave);
	if(pRow == NULL) {
		// (z.manning 2010-09-15 12:41) - Nothing to save
		return TRUE;
	}

	long nMarkupID = VarLong(pRow->GetValue(mccID));

	CString strName, strTemp;
	GetDlgItemText(IDC_MARKUP_NAME, strName);
	strTemp = strName;
	strTemp.TrimRight();
	if(strTemp.IsEmpty()) {
		MessageBox("You must enter a name for this markup.", NULL, MB_OK|MB_ICONERROR);
		return FALSE;
	}

	CString strAdditionalWhere;
	if(!IsNew(pRow)) {
		strAdditionalWhere = FormatString(" AND ID <> %li ", nMarkupID);
	}
	if(ReturnsRecords("SELECT ID FROM MarkupFormulasT WHERE Name = '%s' %s", _Q(strName), strAdditionalWhere)) {
		MessageBox("There is already a markup named " + strName + ".", NULL, MB_OK|MB_ICONERROR);
		return FALSE;
	}

	CString strFormula = GetFormula();

	CNxExpression expression(strFormula);
	expression.AddOperandReplacement(MARKUP_COST_OPERAND, "1");
	expression.Evaluate();
	CString strExpressionError = expression.GetError();
	if(!strExpressionError.IsEmpty()) {
		MessageBox("There is an error in your formula:\r\n\r\n" + strExpressionError, NULL, MB_OK|MB_ICONERROR);
		return FALSE;
	}

	strTemp = strFormula;
	strTemp.TrimRight();
	if(strTemp.IsEmpty()) {
		MessageBox("You must enter a formula.", NULL, MB_OK|MB_ICONERROR);
		return FALSE;
	}

	// (z.manning 2010-09-21 11:48) - Warn if the formula is just the cost, but no reason not to allow it.
	if(strFormula == MARKUP_COST_OPERAND) {
		int nResult = MessageBox("This markup formula is not set to do anything so any items that use it will have their price set as the cost.\r\n\r\n"
			"Are you sure you want to save this markup?", NULL, MB_YESNO|MB_ICONQUESTION);
		if(nResult != IDYES) {
			return FALSE;
		}
	}

	// (b.eyers 2016-03-14) - PLID 68590 - add a dropdown for round up rules
	NXDATALIST2Lib::IRowSettingsPtr pRoundUpRuleRow = m_pRoundUpRule->CurSel;
	long nRoundUp = VarLong(pRoundUpRuleRow->GetValue(rrcID), 0);

	if(IsNew(pRow))
	{
		//r.wilson 3/9/2012 PLID 46664 added the column RoundUp
		ADODB::_RecordsetPtr prsMarkup = CreateParamRecordset(
			"SET NOCOUNT ON \r\n"
			"INSERT INTO MarkupFormulasT (Name, Formula, RoundUpRule) VALUES ({STRING}, {STRING}, {INT}) \r\n"
			"SET NOCOUNT OFF \r\n"
			"SELECT convert(int, SCOPE_IDENTITY()) AS MarkupID \r\n"
			, strName, strFormula, nRoundUp);
		nMarkupID = AdoFldLong(prsMarkup->GetFields(), "MarkupID");
	}
	else
	{
		//r.wilson 3/9/2012 PLID 46664 added the column RoundUp
		ExecuteParamSql("UPDATE MarkupFormulasT SET Name = {STRING}, Formula = {STRING}, RoundUpRule = {INT} WHERE ID = {INT}"
			, strName, strFormula, nRoundUp, nMarkupID);
	}

	pRow->PutValue(mccID, nMarkupID);
	pRow->PutValue(mccName, _bstr_t(strName));
	pRow->PutValue(mccFormula, _bstr_t(strFormula));
	pRow->PutValue(mccRoundUpRule, nRoundUp);
	m_pdlMarkupCombo->Sort();

	return TRUE;
}

BOOL CMarkupFormulaEditDlg::HasRowChanged(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return FALSE;
	}

	CString strCurrentName;
	GetDlgItemText(IDC_MARKUP_NAME, strCurrentName);
	if(strCurrentName != VarString(pRow->GetValue(mccName),"")) {
		return TRUE;
	}

	CString strCurrentFormula = GetFormula();
	if(strCurrentFormula != VarString(pRow->GetValue(mccFormula),"")) {
		return TRUE;
	}

	// (b.eyers 2016-03-14) - PLID 68590 - add a dropdown for round up rules
	NXDATALIST2Lib::IRowSettingsPtr pRoundUpRuleRow = m_pRoundUpRule->CurSel;
	long nCurrentRoundUp = VarLong(pRoundUpRuleRow->GetValue(rrcID), 0);
	if (nCurrentRoundUp != VarLong(pRow->GetValue(mccRoundUpRule), 0)) {
		return TRUE;
	}

	return FALSE;
}

CString CMarkupFormulaEditDlg::GetFormula()
{
	CString strFormula;
	GetDlgItemText(IDC_MARKUP_FORMULA, strFormula);
	strFormula = MARKUP_COST_OPERAND + strFormula;
	return strFormula;
}

void CMarkupFormulaEditDlg::OnBnClickedNewMarkup()
{
	try
	{
		// (z.manning 2010-11-04 09:08) - Prompt to save the previous row first
		IRowSettingsPtr pCurrentRow(m_pdlMarkupCombo->GetCurSel());
		if(pCurrentRow != NULL && HasRowChanged(pCurrentRow)) {
			CString strPreviousMarkup = VarString(pCurrentRow->GetValue(mccName), "");
			int nResult = MessageBox(FormatString("Would you like to save changes to the '%s' markup?", strPreviousMarkup), NULL, MB_YESNOCANCEL|MB_ICONQUESTION);
			if(nResult == IDYES) {
				// (z.manning 2010-11-04 09:08) - They chose to save to attempt to do so but if that fails do not continue.
				if(!ValidateAndSave(pCurrentRow)) {
					return;
				}
			}
			else if(nResult == IDNO) {
				// (z.manning 2010-11-04 09:10) - They don't want to save, but if this was a new markup we need
				// to remove the row.
				if(IsNew(pCurrentRow)) {
					m_pdlMarkupCombo->RemoveRow(pCurrentRow);
				}
			}
			else if(nResult == IDCANCEL) {
				// (z.manning 2010-11-04 09:10) - They cancelled so stay on the same row.
				return;
			}
		}

		IRowSettingsPtr pNewRow = m_pdlMarkupCombo->GetNewRow();
		pNewRow->PutValue(mccID, (long)-1);
		pNewRow->PutValue(mccName, "[New Markup]");
		pNewRow->PutValue(mccFormula, ""); 
		pNewRow->PutValue(mccRoundUpRule, NoRoundUpRule); // (b.eyers 2016-03-14) - PLID 68590 - default to no round up rule
		m_pdlMarkupCombo->PutCurSel(m_pdlMarkupCombo->AddRowSorted(pNewRow, NULL));
		Load();
		((CEdit*)GetDlgItem(IDC_MARKUP_NAME))->SetSel(0, -1);
		GetDlgItem(IDC_MARKUP_NAME)->SetFocus();

	}NxCatchAll(__FUNCTION__);
}

void CMarkupFormulaEditDlg::OnBnClickedDeleteMarkup()
{
	try
	{
		IRowSettingsPtr pRow(m_pdlMarkupCombo->GetCurSel());
		if(pRow == NULL) {
			return;
		}

		CString strName = VarString(pRow->GetValue(mccName), "");
		int nResult = MessageBox("Are you sure you want to delete the '" + strName + "' markup? "
			"Deleting it will also remove it from any inventory products or items in the frames catalog.", NULL, MB_YESNO|MB_ICONQUESTION);
		if(nResult != IDYES) {
			return;
		}

		if(!IsNew(pRow)) {
			long nMarkupID = VarLong(pRow->GetValue(mccID), -1);
			CParamSqlBatch sqlBatch;
			sqlBatch.Add("DELETE FROM FramesMarkupLinkT WHERE MarkupFormulaID = {INT}", nMarkupID);
			sqlBatch.Add("DELETE FROM MarkupFormulasT WHERE ID = {INT}", nMarkupID);
			sqlBatch.Execute(GetRemoteData());
		}

		m_pdlMarkupCombo->RemoveRow(pRow);
		m_pdlMarkupCombo->PutCurSel(m_pdlMarkupCombo->GetFirstRow());
		Load();

	}NxCatchAll(__FUNCTION__);
}

BOOL CMarkupFormulaEditDlg::IsNew(LPDISPATCH lpRow)
{
	IRowSettingsPtr pRow(lpRow);
	if(pRow == NULL) {
		return FALSE;
	}

	long nMarkupID = VarLong(pRow->GetValue(mccID), -1);
	return (nMarkupID == -1);
}

void CMarkupFormulaEditDlg::OnOK()
{
	try
	{
		if(ValidateAndSave(m_pdlMarkupCombo->GetCurSel())) {			
			CNxDialog::OnOK();
		}

	}NxCatchAll(__FUNCTION__);
}

void CMarkupFormulaEditDlg::OnBnClickedMarkupHelp()
{
	try
	{
		MessageBox(
			"In this field you can enter a formula for the markup price relative to the cost.\r\n\r\n"
			"For example, if you want the markup price to be 2 times the cost plus $50 then enter:\r\n\r\n"
			"\t * 2 + 50\r\n\r\n"
			"as the markup formula."
			, NULL, MB_OK|MB_ICONINFORMATION);

	}NxCatchAll(__FUNCTION__);
}

// (b.eyers 2016-03-15) - PLID 68590 
void CMarkupFormulaEditDlg::OnBnClickedRoundupHelp()
{
	try
	{
		//Round Sales Price to nearest dollar
		//Round Sales Price up to next 9
		MessageBox(
			"Round Sales Price up to nearest dollar: $135.25 would round to $136\r\n\r\n"
			"Round Sales Price up to next 9: $135.25 would round to $139\r\n\r\n"
			, NULL, MB_OK | MB_ICONINFORMATION);

	}NxCatchAll(__FUNCTION__);
}