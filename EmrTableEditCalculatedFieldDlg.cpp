// EmrTableEditCalculatedFieldDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "EmrTableEditCalculatedFieldDlg.h"
#include "NxExpression.h"
#include "EMRUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

// (z.manning, 05/30/2008) - PLID 16443 - Added global function to get a column's operand based on its index
CString GetColumnOperand(long nColIndex)
{
	return 'C' + AsString(nColIndex);
}
CString GetRowOperand(long nRowIndex)
{
	return 'R' + AsString(nRowIndex);
}

// (z.manning 2008-06-05 17:18) - PLID 30155 - Returns the index of the row or column or -1 if invalid
long GetRowIndexFromOperand(CString strOperand)
{
	if(strOperand.GetLength() == 0) {
		return -1;
	}

	if(strOperand.GetAt(0) == 'R') {
		strOperand.Delete(0);
		long nIndex = atol(strOperand);
		if(nIndex != 0) {
			return nIndex;
		}
	}

	return -1;
}
short GetColumnIndexFromOperand(CString strOperand)
{
	if(strOperand.GetLength() == 0) {
		return -1;
	}

	if(strOperand.GetAt(0) == 'C') {
		strOperand.Delete(0);
		long nIndex = atol(strOperand);
		if(nIndex != 0) {
			return (short)nIndex;
		}
	}

	return -1;
}

// (z.manning, 05/30/2008) - PLID 16443 - Returns true if the specified element exists in the formula
// of any calculated field, false if it does not.
// (z.manning 2008-06-05 11:05) - PLID 30145 - Move here from CEmrItemEntryDlg and made it global
BOOL IsDataElementReferencedInAnyFormula(CEmrInfoDataElement *peide, CEmrInfoDataElementArray &aryRows, CEmrInfoDataElementArray &aryColumns, OUT CString &strReferencingItem)
{
	long nRow, nCol;
	// (z.manning 2008-06-05 09:10) - First check any column formulas
	for(nCol = 0; nCol < aryColumns.GetSize(); nCol++)
	{
		CEmrInfoDataElement *pTemp = aryColumns.GetAt(nCol);
		if(IsDataElementReferencedInElement(peide, pTemp)) {
			strReferencingItem = pTemp->m_strData;
			return TRUE;
		}
	}
	// (z.manning 2008-06-05 09:11) - Now check row formulas
	for(nRow = 0; nRow < aryRows.GetSize(); nRow++)
	{
		CEmrInfoDataElement *pTemp = aryRows.GetAt(nRow);
		if(IsDataElementReferencedInElement(peide, pTemp)) {
			strReferencingItem = pTemp->m_strData;
			return TRUE;
		}
	}

	return FALSE;
}

BOOL IsDataElementReferencedInElement(CEmrInfoDataElement *peidePotentialReference, const CString strFormula)
{
	if(strFormula.IsEmpty()) {
		return FALSE;
	}

	CString strOperand;
	if(peidePotentialReference->m_nListType >= 3) {
		strOperand = GetColumnOperand(peidePotentialReference->m_nVisibleIndex);
	}
	else {
		strOperand = GetRowOperand(peidePotentialReference->m_nVisibleIndex);
	}

	CNxExpression expression(strFormula);
	if(expression.HasOperand(strOperand)) {
		return TRUE;
	}
	else {
		return FALSE;
	}
}

BOOL IsDataElementReferencedInElement(CEmrInfoDataElement *peidePotentialReference, CEmrInfoDataElement *peideFormula)
{
	// (z.manning 2011-05-31 10:49) - PLID 42131 - Don't count tranformation formulas.
	if((peideFormula->m_nFlags & edfFormulaForTransform) != 0) {
		return FALSE;
	}
	if((peidePotentialReference->m_nFlags & edfFormulaForTransform) != 0) {
		return FALSE;
	}

	return IsDataElementReferencedInElement(peidePotentialReference, peideFormula->m_strFormula);
}


// (z.manning, 05/22/2008) - PLID 30145 - Created
/////////////////////////////////////////////////////////////////////////////
// CEmrTableEditCalculatedFieldDlg dialog

// (j.jones 2011-07-08 13:18) - PLID 43032 - added eDataSubType
CEmrTableEditCalculatedFieldDlg::CEmrTableEditCalculatedFieldDlg(CEmrInfoDataElementArray &aryRows, CEmrInfoDataElementArray &aryColumns,
																 CEmrItemEntryDlg* pParent, EmrInfoSubType eDataSubType)
	: CNxDialog(CEmrTableEditCalculatedFieldDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEmrTableEditCalculatedFieldDlg)
	m_nDecimalPlaces = 0;
	m_strFormula = _T("");
	//}}AFX_DATA_INIT

	m_aryRows.AppendCopy(aryRows);
	m_aryColumns.AppendCopy(aryColumns);
	m_nEditingRowSortOrder = -1;
	m_nEditingColumnSortOrder = -1;
	m_nCaretIndex = 0;
	m_pEmrItemEntryDlg = pParent;
	m_pButtonFont = NULL;
	m_bTableRowsAsFields = FALSE;
	// (j.jones 2011-07-08 13:18) - PLID 43032 - added m_DataSubType
	m_DataSubType = eDataSubType;
}

CEmrTableEditCalculatedFieldDlg::~CEmrTableEditCalculatedFieldDlg()
{
	if(m_pButtonFont != NULL) {
		delete m_pButtonFont;
		m_pButtonFont = NULL;
	}
}


void CEmrTableEditCalculatedFieldDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEmrTableEditCalculatedFieldDlg)
	DDX_Control(pDX, IDC_FORMULA, m_richeditFormula);
	DDX_Control(pDX, IDC_TEST_FORMULA, m_btnTestFormula);
	DDX_Control(pDX, IDC_PLUS, m_btnPlus);
	DDX_Control(pDX, IDC_PARENTHESES, m_btnParentheses);
	DDX_Control(pDX, IDC_NUMBER_9, m_btnNumber9);
	DDX_Control(pDX, IDC_NUMBER_8, m_btnNumber8);
	DDX_Control(pDX, IDC_NUMBER_7, m_btnNumber7);
	DDX_Control(pDX, IDC_NUMBER_6, m_btnNumber6);
	DDX_Control(pDX, IDC_NUMBER_5, m_btnNumber5);
	DDX_Control(pDX, IDC_NUMBER_4, m_btnNumber4);
	DDX_Control(pDX, IDC_NUMBER_3, m_btnNumber3);
	DDX_Control(pDX, IDC_NUMBER_2, m_btnNumber2);
	DDX_Control(pDX, IDC_NUMBER_1, m_btnNumber1);
	DDX_Control(pDX, IDC_NUMBER_0, m_btnNumber0);
	DDX_Control(pDX, IDC_MULTIPLY, m_btnMultiply);
	DDX_Control(pDX, IDC_MINUS, m_btnMinus);
	DDX_Control(pDX, IDC_EXPONENT, m_btnExponent);
	DDX_Control(pDX, IDC_DOT, m_btnDot);
	DDX_Control(pDX, IDC_DIVIDE, m_btnDivide);
	DDX_Control(pDX, IDC_CLEAR_FORMULA, m_btnClearFormula);
	DDX_Control(pDX, IDC_BACKSPACE, m_btnBackspace);
	DDX_Control(pDX, IDC_MODULO, m_btnModulo);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDC_FORMULA_LABEL, m_nxstaticFormulaLabel);
	DDX_Control(pDX, IDC_DECIMAL_PLACES, m_nxeditDecimalPlaces);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Text(pDX, IDC_DECIMAL_PLACES, m_nDecimalPlaces);
	DDV_MinMaxUInt(pDX, m_nDecimalPlaces, 0, 255);
	DDX_Text(pDX, IDC_FORMULA, m_strFormula);
	DDV_MaxChars(pDX, m_strFormula, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEmrTableEditCalculatedFieldDlg, CNxDialog)
	//{{AFX_MSG_MAP(CEmrTableEditCalculatedFieldDlg)
	ON_BN_CLICKED(IDC_TEST_FORMULA, OnTestFormula)
	ON_BN_CLICKED(IDC_DIVIDE, OnDivide)
	ON_BN_CLICKED(IDC_EXPONENT, OnExponent)
	ON_BN_CLICKED(IDC_MINUS, OnMinus)
	ON_BN_CLICKED(IDC_PARENTHESES, OnParentheses)
	ON_BN_CLICKED(IDC_PLUS, OnPlus)
	ON_BN_CLICKED(IDC_MULTIPLY, OnMultiply)
	ON_BN_CLICKED(IDC_CLEAR_FORMULA, OnClearFormula)
	ON_BN_CLICKED(IDC_DOT, OnDot)
	ON_BN_CLICKED(IDC_NUMBER_0, OnNumber0)
	ON_BN_CLICKED(IDC_NUMBER_1, OnNumber1)
	ON_BN_CLICKED(IDC_NUMBER_2, OnNumber2)
	ON_BN_CLICKED(IDC_NUMBER_3, OnNumber3)
	ON_BN_CLICKED(IDC_NUMBER_4, OnNumber4)
	ON_BN_CLICKED(IDC_NUMBER_5, OnNumber5)
	ON_BN_CLICKED(IDC_NUMBER_6, OnNumber6)
	ON_BN_CLICKED(IDC_NUMBER_7, OnNumber7)
	ON_BN_CLICKED(IDC_NUMBER_8, OnNumber8)
	ON_BN_CLICKED(IDC_NUMBER_9, OnNumber9)
	ON_NOTIFY(EN_SELCHANGE, IDC_FORMULA, OnSelchangeFormula)
	ON_BN_CLICKED(IDC_BACKSPACE, OnBackspace)
	ON_EN_CHANGE(IDC_FORMULA, OnChangeFormula)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_MODULO, &CEmrTableEditCalculatedFieldDlg::OnBnClickedModulo)
	ON_BN_CLICKED(IDC_FORMULA_FOR_TRANSFORM, &CEmrTableEditCalculatedFieldDlg::OnBnClickedFormulaForTransform)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEmrTableEditCalculatedFieldDlg message handlers

BOOL CEmrTableEditCalculatedFieldDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		m_pButtonFont = new CFont;
		CreateCompatiblePointFont(m_pButtonFont, 140, "Arial Bold");

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (z.manning 2008-06-09 12:29) - Color the buttons similar to Windows calculator
		const COLORREF clrOperandColor = RGB(0, 0, 255);
		const COLORREF clrOperatorColor = RGB(255, 0, 0);
		m_btnPlus.SetTextColor(clrOperatorColor);
		m_btnParentheses.SetTextColor(clrOperatorColor);
		m_btnNumber9.SetTextColor(clrOperandColor);
		m_btnNumber8.SetTextColor(clrOperandColor);
		m_btnNumber7.SetTextColor(clrOperandColor);
		m_btnNumber6.SetTextColor(clrOperandColor);
		m_btnNumber5.SetTextColor(clrOperandColor);
		m_btnNumber4.SetTextColor(clrOperandColor);
		m_btnNumber3.SetTextColor(clrOperandColor);
		m_btnNumber2.SetTextColor(clrOperandColor);
		m_btnNumber1.SetTextColor(clrOperandColor);
		m_btnNumber0.SetTextColor(clrOperandColor);
		m_btnMultiply.SetTextColor(clrOperatorColor);
		m_btnMinus.SetTextColor(clrOperatorColor);
		m_btnExponent.SetTextColor(clrOperatorColor);
		m_btnDot.SetTextColor(clrOperandColor);
		m_btnDivide.SetTextColor(clrOperatorColor);
		m_btnBackspace.SetTextColor(clrOperatorColor);
		m_btnModulo.SetTextColor(clrOperatorColor); // (z.manning 2011-05-26 10:24) - PLID 43851
		// (z.manning 2008-06-10 08:38) - Set button fonts
		m_btnPlus.SetFont(m_pButtonFont);
		m_btnParentheses.SetFont(m_pButtonFont);
		m_btnNumber9.SetFont(m_pButtonFont);
		m_btnNumber8.SetFont(m_pButtonFont);
		m_btnNumber7.SetFont(m_pButtonFont);
		m_btnNumber6.SetFont(m_pButtonFont);
		m_btnNumber5.SetFont(m_pButtonFont);
		m_btnNumber4.SetFont(m_pButtonFont);
		m_btnNumber3.SetFont(m_pButtonFont);
		m_btnNumber2.SetFont(m_pButtonFont);
		m_btnNumber1.SetFont(m_pButtonFont);
		m_btnNumber0.SetFont(m_pButtonFont);
		m_btnMultiply.SetFont(m_pButtonFont);
		m_btnMinus.SetFont(m_pButtonFont);
		m_btnExponent.SetFont(m_pButtonFont);
		m_btnDot.SetFont(m_pButtonFont);
		m_btnDivide.SetFont(m_pButtonFont);

		// (z.manning 2008-06-10 09:51) - Call this so that we receive sel changed events for the formula field.
		m_richeditFormula.SendMessage(EM_SETEVENTMASK, 0, ENM_SELCHANGE|ENM_CHANGE);

		// (z.manning 2010-11-29 14:38) - PLID 39025 - We need to make sure these arrays are in the same order
		// in this array. We already have them in order of their sort orders, but sort order doesn't matter if
		// the auto-alphabatize option is enabled.
		qsort(m_aryRows.GetData(), m_aryRows.GetSize(), sizeof(CEmrInfoDataElement*), CompareInfoDataElementsByVisibleIndex);
		qsort(m_aryColumns.GetData(), m_aryColumns.GetSize(), sizeof(CEmrInfoDataElement*), CompareInfoDataElementsByVisibleIndex);

		m_pdlTable = BindNxDataList2Ctrl(IDC_SAMPLE_TABLE, GetRemoteData(), false);
		m_pdlTable->PutGridVisible(VARIANT_TRUE);
		m_pdlTable->PutHighlightVisible(VARIANT_FALSE);

		// (z.manning, 06/04/2008) - Insert the row title column
		// (j.luckoski 2012-08-31 13:54) - PLID 51755 - set the columns to fixed width so no one could manually resize.
		m_pdlTable->InsertColumn(0, _T(""), _T(""), 0, csVisible | csFixedWidth);
		
		// (c.haag 2008-10-23 12:42) - PLID 31834 - Table flipping support
		int nTableRows = (m_bTableRowsAsFields) ? m_aryColumns.GetSize() : m_aryRows.GetSize();
		int nTableColumns = (m_bTableRowsAsFields) ? m_aryRows.GetSize() : m_aryColumns.GetSize();

		short nColIndex;
		for(nColIndex = 0; nColIndex < nTableColumns; nColIndex++)
		{
			// (c.haag 2008-10-27 11:19) - PLID 31834 - Support for table flipping
			CEmrInfoDataElement *pColumnInfo = (m_bTableRowsAsFields) ? m_aryRows.GetAt(nColIndex) : m_aryColumns.GetAt(nColIndex);
			if(!pColumnInfo->m_bInactive) {
				// (j.luckoski 2012-08-31 13:57) - PLID 51755 - Set fixed width
				IColumnSettingsPtr pCol = m_pdlTable->GetColumn(m_pdlTable->InsertColumn(nColIndex + 1, _T(""), _T(""), 0, csVisible | csFixedWidth));
				pCol->FieldType = cftTextSingleLine;
				pCol->DataType = VT_BSTR;
			}
			else {
				// (z.manning, 06/04/2008) - Get rid of any inactive columns for this dialog
				// (c.haag 2008-10-30 16:37) - PLID 31834 - Support for table flipping
				if (m_bTableRowsAsFields) {
					delete m_aryRows.GetAt(nColIndex);
					m_aryRows.RemoveAt(nColIndex);
				} else {
					delete m_aryColumns.GetAt(nColIndex);
					m_aryColumns.RemoveAt(nColIndex);
				}
				nColIndex--;
				nTableColumns--;
			}
		}

		// (z.manning, 05/22/2008) - Add the row AFTER adding the columns because the datalist RowSettingsPtr
		// doesn't seem to handle dynamic column adding/removing well.
		IRowSettingsPtr pTitleRow = m_pdlTable->GetNewRow();
		m_pdlTable->AddRowAtEnd(pTitleRow, NULL);
		_variant_t varNull;
		varNull.vt = VT_NULL;
		pTitleRow->PutValue(0, varNull);

		// (j.luckoski 2012-08-30 14:18) - PLID 51755 - Handle the width so we can add more to the height if the scrollbar is appearing.
		m_nIdealTableWidth = 0;

		// (c.haag 2008-10-23 12:42) - PLID 31834 - Table flipping support
		for(nColIndex = 0; nColIndex < nTableColumns; nColIndex++)
		{
			// (c.haag 2008-10-23 12:39) - PLID 31834 - Consider table flipping
			CEmrInfoDataElement *pColumnData = (m_bTableRowsAsFields) ? m_aryRows.GetAt(nColIndex) : m_aryColumns.GetAt(nColIndex);
			pTitleRow->PutValue(nColIndex + 1, _bstr_t(pColumnData->m_strData));
			pTitleRow->PutCellLinkStyle(nColIndex + 1, dlLinkStyleTrue);
			pTitleRow->PutCellForeColor(nColIndex + 1, RGB(0,0,200));
			// (z.manning, 05/22/2008) - Auto-resize each active column to the size of the column title.
			IColumnSettingsPtr pCol = m_pdlTable->GetColumn(nColIndex + 1);
			long nIdealColumnWidth = pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE);
			pCol->PutStoredWidth(nIdealColumnWidth);
			// (j.luckoski 2012-08-30 14:18) - PLID 51755 - Add the width to the table width
			m_nIdealTableWidth += nIdealColumnWidth;

			// (j.luckoski 2012-08-01 10:02) - PLID 51755 - Add simple column title's to the column headers.
			CString strTempLabel = "";
			strTempLabel.Format("C%i", nColIndex + 1);
			pCol->ColumnTitle = (_bstr_t)strTempLabel;
		}

		long nIdealTableHeight = pTitleRow->GetHeight();

		for(int nRowIndex = 0; nRowIndex < nTableRows; nRowIndex++)
		{
			// (c.haag 2008-10-23 12:39) - PLID 31834 - Consider table flipping
			CEmrInfoDataElement *pRowData = (m_bTableRowsAsFields) ? m_aryColumns.GetAt(nRowIndex) : m_aryRows.GetAt(nRowIndex);
			if(pRowData->m_bInactive) {
				if (m_bTableRowsAsFields) {
					delete m_aryColumns.GetAt(nRowIndex);
					m_aryColumns.RemoveAt(nRowIndex);
				} else {
					delete m_aryRows.GetAt(nRowIndex);
					m_aryRows.RemoveAt(nRowIndex);
				}
				nRowIndex--;
				nTableRows--;
				continue;
			}
			IRowSettingsPtr pRow = m_pdlTable->GetNewRow();
			pRow->PutValue(0, _bstr_t(pRowData->m_strData));
			pRow->PutCellLinkStyle(0, dlLinkStyleTrue);
			pRow->PutCellForeColor(0, RGB(0,0,200));

			m_pdlTable->AddRowAtEnd(pRow, NULL);
			nIdealTableHeight += pRow->GetHeight();
		}

		// (j.luckoski 2012-08-27 16:41) - PLID 51755 - I had to increase the ideal table height in order to fit the rows with the column header.
		nIdealTableHeight += 23;
		// (z.manning 2008-06-06 15:50) - Handle the row title column
		IColumnSettingsPtr pFirstColumn = m_pdlTable->GetColumn(0);
		long nColWidth = pFirstColumn->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE);
		pFirstColumn->PutStoredWidth(nColWidth);

		// (z.manning, 05/22/2008) - Auto-resize the dialog's height based on the ideal table height
		{
			CRect rcDialog, rcDatalist;
			GetWindowRect(rcDialog);
			ScreenToClient(rcDialog);
			CWnd *pwndDatalist = GetDlgItem(IDC_SAMPLE_TABLE);
			pwndDatalist->GetWindowRect(rcDatalist);
			ScreenToClient(rcDatalist);
			// (j.luckoski 2012-08-30 14:19) - PLID 51755 - If the scrollbar is going to appear on the bottom, add some height 
			if(m_nIdealTableWidth >= (rcDatalist.Width() - 49)) {
					nIdealTableHeight += 19;
			}
			if(nIdealTableHeight < rcDatalist.Height())
			{
				int nVerticalShrink = rcDatalist.Height() - nIdealTableHeight;
				pwndDatalist->MoveWindow(rcDatalist.left, rcDatalist.top, rcDatalist.Width(), rcDatalist.Height() - nVerticalShrink);
				MoveWindow(rcDialog.left, rcDialog.top, rcDialog.Width(), rcDialog.Height() - nVerticalShrink);
				CenterWindow();
			}
		}

		CEmrInfoDataElement *peideCurrent = GetCurrentData();
		if(peideCurrent != NULL && !CanHaveFormula(peideCurrent, TRUE)) {
			m_nEditingRowSortOrder = m_nEditingColumnSortOrder = -1;
		}

		// (j.luckoski 2012-08-31 14:00) - PLID 51755 - we want the update to happen on first load but not the resize til after the first update
		m_bRunResize = false;

		UpdateView(TRUE);

		m_bRunResize = true;
	
	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

CString CEmrTableEditCalculatedFieldDlg::GetFormula()
{
	if(IsWindow(GetSafeHwnd())) {
		UpdateData(TRUE);
		m_strFormula.MakeUpper();
		ReflectCurrentFormula();
		// (j.luckoski 2012-08-31 14:00) - PLID 51755 - Resize height with new column widths
		ResizeHeight();
	}

	return m_strFormula;
}

void CEmrTableEditCalculatedFieldDlg::OnTestFormula() 
{
	try
	{
		if(TestFormula()) {
			Save();
			((CEmrItemEntryDlg*)GetParent())->PreviewTable(&m_aryRows, &m_aryColumns);
		}

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnTestFormula");
}

BOOL CEmrTableEditCalculatedFieldDlg::TestFormula()
{
	// (z.manning, 05/29/2008) - Ensure m_strFormula is up to date.
	GetFormula();

	// (z.manning 2008-06-05 10:25) - Empty formulas are fine.
	CString strTemp = m_strFormula;
	strTemp.TrimRight();
	if(strTemp.IsEmpty()) {
		m_strFormula.Empty();
		ReflectCurrentFormula();
		// (j.luckoski 2012-08-31 14:00) - PLID 51755 - Resize height with new column widths
		ResizeHeight();
		return TRUE;
	}

	// (r.gonet 08/03/2012) - PLID 51952 - Get the operand name from the index
	CString strCurrentField;
	if(m_nEditingColumnSortOrder >= 0) {
		strCurrentField = GetColumnOperand(m_nEditingColumnSortOrder);
	} else if(m_nEditingRowSortOrder >= 0) {
		strCurrentField = GetRowOperand(m_nEditingRowSortOrder);
	}
	// (r.gonet 08/03/2012) - PLID 51952 - Pass the field as well as the formula so that we can use the name to detect circular references.
	CNxExpression expression(strCurrentField, m_strFormula);
	// (z.manning, 05/29/2008) - For testing purposes, replace all valid operands with a value of 1
	// (c.haag 2008-11-04 10:49) - PLID 31834 - Change the entire iteration ordering to support table flipping
	IRowSettingsPtr pRow = m_pdlTable->GetFirstRow();
	pRow = pRow->GetNextRow();
	int nTableRows = m_pdlTable->GetRowCount();
	int nTableColumns = m_pdlTable->GetColumnCount();
	int iTableRow = 1;
	int iTableColumn = 1;
	BOOL bIsTransform = IsDlgButtonChecked(IDC_FORMULA_FOR_TRANSFORM) == BST_CHECKED;

	if(m_nEditingRowSortOrder >= 0 || m_nEditingColumnSortOrder >= 0) {
		while (iTableRow < nTableRows && iTableColumn < nTableColumns) {
			CString strCellOperand = VarString(pRow->GetValue(iTableColumn), "");
			// (c.haag 2008-11-04 10:58) - PLID 31834 - Determine how to iterate based on several factors
			BOOL bIterateByRow = (m_nEditingRowSortOrder >= 0) ? TRUE : FALSE;
			if (m_bTableRowsAsFields) {
				bIterateByRow = (bIterateByRow) ? FALSE : TRUE;
			}
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method instead of the index property
			// (c.haag 2008-11-04 11:10) - PLID 31834 - Changed to support table flipping
			if(!strCellOperand.IsEmpty()) {
				// (z.manning 2011-05-31 10:37) - PLID 42131 - Transformation formulas can reference themselves
				if (bIterateByRow && (iTableRow != GetCurrentData()->m_nVisibleIndex || bIsTransform)) {
					expression.AddOperandReplacement(m_bTableRowsAsFields ? GetColumnOperand(iTableRow) : GetRowOperand(iTableRow), "1");
				}
				// (z.manning 2011-05-31 10:37) - PLID 42131 - Transformation formulas can reference themselves
				else if (!bIterateByRow && (iTableColumn != GetCurrentData()->m_nVisibleIndex || bIsTransform)) {
					expression.AddOperandReplacement(m_bTableRowsAsFields ? GetRowOperand(iTableColumn) : GetColumnOperand(iTableColumn), "1");
				}
			}
			// (c.haag 2008-11-04 11:08) - PLID 31834 - Now go to the next element
			if (bIterateByRow) {
				iTableRow++;
				pRow = pRow->GetNextRow();
			} else {
				iTableColumn++;
			}
		}
	} else {
		ASSERT(FALSE);
	}

	// (r.gonet 08/03/2012) - PLID 51952 - Gather all of the formulas from other fields and add them to the expression so it
	//  can flatten its own expression through substitution.
	if(!bIsTransform) {
		if(m_nEditingColumnSortOrder >= 0) {
			for(int nCol = 0; nCol < m_aryColumns.GetSize(); nCol++) {
				CEmrInfoDataElement *pColData = m_aryColumns.GetAt(nCol);
				if(pColData->m_nSortOrder > 0 && m_nEditingColumnSortOrder != pColData->m_nSortOrder) {
					CString strField = GetColumnOperand(pColData->m_nSortOrder);
					CString strExpression = pColData->m_strFormula;
					if(!strExpression.IsEmpty() && (pColData->m_nFlags & edfFormulaForTransform) == 0) {
						expression.AddFormula(strField, strExpression);
					} else {
						// (r.gonet 08/03/2012) - PLID 51952 - The field references a column that is filled by the user. We don't add this to the formulas
						//  for two reasons, 1) It would flatten to an invalid expression eg 'C1 + C2 + 4' = ' +  + 4' and
						//  2) CNxExpression already works with formulas that contain fields that are user filled.
						// Or this could be a transform field rather than a regular formula field. 
						//  Transform fields can be self referencing and are evaluated on current data that has already been
						//  calculated rather than recursively going through formulas.
					}
				}
			}
		}
		else if(m_nEditingRowSortOrder >= 0) {
			for(int nRow = 0; nRow < m_aryRows.GetSize(); nRow++) {
				CEmrInfoDataElement *pRowData = m_aryRows.GetAt(nRow);
				if(pRowData->m_nSortOrder > 0 && m_nEditingRowSortOrder != pRowData->m_nSortOrder) {
					CString strField = GetRowOperand(pRowData->m_nSortOrder);
					CString strExpression = pRowData->m_strFormula;
					if(!strExpression.IsEmpty() && (pRowData->m_nFlags & edfFormulaForTransform) == 0) {
						expression.AddFormula(strField, strExpression);
					} else if(pRowData) {
						// (r.gonet 08/03/2012) - PLID 51952 - The field references a row that is filled by the user. We don't add this to the formulas
						//  for two reasons, 1) It would flatten to an invalid expression eg 'C1 + C2 + 4' = ' +  + 4' and
						//  2) CNxExpression works with formulas already that contain fields that are user filled.
						// Or this could be a transform only field. Transform only fields are evaluated after all other formulas
						//  have been evaluated, so we can't reference one in a non-transform formula
					}
				}
			}
		}
	} else {
		// (r.gonet 08/03/2012) - PLID 51952 - The formula we are testing is a tranform formula, which has a different evaluation order than regular formulas.
		//  We don't want to flatten it because it will be evaluated on the table's current data for each cell it references.
	}

	// (c.haag 2008-11-04 11:29) - PLID 31834 - Legacy code for comparison
	/*
	if(m_nEditingRowSortOrder >= 0)
	{
		IRowSettingsPtr pRow = m_pdlTable->GetFirstRow();
		pRow = pRow->GetNextRow();
		for(; pRow != NULL; pRow = pRow->GetNextRow()) {
			CString strCellOperand = VarString(pRow->GetValue(1), "");
			// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method instead of the index property
			if(!strCellOperand.IsEmpty() && pRow->CalcRowNumber() != GetCurrentData()->m_nVisibleIndex) {
				expression.AddOperandReplacement(GetRowOperand(pRow->CalcRowNumber()), "1");
			}
		}
	}
	else
	{
		ASSERT(m_nEditingColumnSortOrder >= 0);
		IRowSettingsPtr pOperandRow = m_pdlTable->GetFirstRow();
		pOperandRow = pOperandRow->GetNextRow();
		if(pOperandRow == NULL) {
			ASSERT(FALSE);
			ThrowNxException("CEmrTableEditCalculatedFieldDlg::TestFormula - No operand row");
		}
		for(short nCol = 1; nCol < m_pdlTable->GetColumnCount(); nCol++) {
			CString strCellOperand = VarString(pOperandRow->GetValue(nCol), "");
			if(!strCellOperand.IsEmpty() && nCol != GetCurrentData()->m_nVisibleIndex) {
				expression.AddOperandReplacement(GetColumnOperand(nCol), "1");
			}
		}
	}*/

	expression.Evaluate();
	CString strError = expression.GetError();
	if(!strError.IsEmpty()) {
		CString strMessage = "Your formula has an error...\r\n\r\n" + strError;
		MessageBox(strMessage, "Error", MB_OK|MB_ICONERROR);
		return FALSE;
	}

	return TRUE;
}

void CEmrTableEditCalculatedFieldDlg::UpdateView(BOOL bReload)
{
	UpdateData(TRUE);
	if(bReload) {
		if(GetCurrentData() != NULL) {
			SetReadOnly(FALSE);
			m_strFormula = GetCurrentData()->m_strFormula;
			m_nDecimalPlaces = GetCurrentData()->m_nDecimalPlaces;
			m_nxstaticFormulaLabel.SetWindowText(GetCurrentData()->m_strData + " =");
			// (z.manning 2011-05-26 15:34) - PLID 43865 - Handle new options
			CheckDlgButton(IDC_FORMULA_SHOW_PLUS_SIGN, (GetCurrentData()->m_nFlags & edfShowPlusSign) != 0 ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(IDC_FORMULA_FOR_TRANSFORM, (GetCurrentData()->m_nFlags & edfFormulaForTransform) != 0 ? BST_CHECKED : BST_UNCHECKED);
		}
		else {
			SetReadOnly(TRUE);
			m_strFormula.Empty();
			m_nDecimalPlaces = 0;
			m_nxstaticFormulaLabel.SetWindowText("(Click on a row or column title to edit its formula)");
			CheckDlgButton(IDC_FORMULA_SHOW_PLUS_SIGN, BST_UNCHECKED);
			CheckDlgButton(IDC_FORMULA_FOR_TRANSFORM, BST_UNCHECKED);
		}
	}

	IRowSettingsPtr pDatalistRow = m_pdlTable->GetFirstRow();
	// (c.haag 2008-10-23 12:52) - PLID 31834 - Get table-flipping-friendly metrics
	int nDetailRows = m_aryRows.GetSize();
	int nDetailColumns = m_aryColumns.GetSize();
	int iDetailRow, iDetailCol;

	// (c.haag 2008-10-27 12:20) - PLID 31834 - Whiten all the first cell backgrounds now
	while (NULL != pDatalistRow) {
		pDatalistRow->PutCellBackColor(0, RGB(255,255,255));
		pDatalistRow = pDatalistRow->GetNextRow();
	}
	pDatalistRow = m_pdlTable->GetFirstRow();

	for(iDetailRow = 0; iDetailRow < nDetailRows; iDetailRow++)
	{
		CEmrInfoDataElement *pRowData = m_aryRows.GetAt(iDetailRow);

		// (c.haag 2008-10-27 11:03) - PLID 31834 - Update the datalist row
		if (m_bTableRowsAsFields) {
			pDatalistRow = m_pdlTable->GetFirstRow();
		} else {
			pDatalistRow = pDatalistRow->GetNextRow();
		}

		for(iDetailCol = 0; iDetailCol < nDetailColumns; iDetailCol++)
		{
			short iDatalistCol;
			if (m_bTableRowsAsFields) { 
				pDatalistRow = pDatalistRow->GetNextRow();
				iDatalistCol = (short)(iDetailRow + 1);
			} else {
				iDatalistCol = (short)(iDetailCol + 1);
			}
			if(pDatalistRow == NULL) {
				AfxThrowNxException("CEmrTableEditCalculatedFieldDlg::UpdateView - NULL row");
			}

			CEmrInfoDataElement *pColumnData = m_aryColumns.GetAt(iDetailCol);
			m_pdlTable->GetFirstRow()->PutCellBackColor(iDatalistCol, RGB(255,255,255));
			if(m_nEditingColumnSortOrder >= 0)
			{
				if(pColumnData->m_nSortOrder == m_nEditingColumnSortOrder) {
					if (m_bTableRowsAsFields) {
						// (c.haag 2008-10-27 11:41) - PLID 31834 - If the table is flipped, we
						// actually want the first column of this row to be green
						pDatalistRow->PutCellBackColor(0, RGB(0,255,0));
					} else {
						m_pdlTable->GetFirstRow()->PutCellBackColor(iDatalistCol, RGB(0,255,0));
					}
					pDatalistRow->PutCellBackColor(iDatalistCol, RGB(0,255,0));
					pDatalistRow->PutCellForeColor(iDatalistCol, RGB(0,0,0));
					// (z.manning 2011-05-31 09:47) - PLID 42131 - A formula can reference itself if it's for a transformation.
					if(iDetailRow == 0 && IsDlgButtonChecked(IDC_FORMULA_FOR_TRANSFORM) == BST_CHECKED) {
						pDatalistRow->PutValue(iDatalistCol, _bstr_t(GetColumnOperand((long)(iDetailCol+1))));
						if(IsDataElementReferencedInElement(pColumnData, m_strFormula)) {
							pDatalistRow->PutCellForeColor(iDatalistCol, RGB(0,32,0));
						}
						else {
							pDatalistRow->PutCellForeColor(iDatalistCol, RGB(255,0,0));
						}
					}
					else {
						pDatalistRow->PutValue(iDatalistCol, "");
						pDatalistRow->PutCellForeColor(iDatalistCol, RGB(0,0,0));
					}
				}
				else {
					if( CanItemBeReferencedInFormulaForItem(pColumnData, GetCurrentData()) ) {
						pDatalistRow->PutCellBackColor(iDatalistCol, RGB(255,255,255));
						if(iDetailRow == 0) {
							pDatalistRow->PutValue(iDatalistCol, _bstr_t(GetColumnOperand((long)(iDetailCol+1))));
							if(IsDataElementReferencedInElement(pColumnData, m_strFormula)) {
								pDatalistRow->PutCellForeColor(iDatalistCol, RGB(0,127,0));
							}
							else {
								pDatalistRow->PutCellForeColor(iDatalistCol, RGB(255,0,0));
							}
						}
						else {
							pDatalistRow->PutValue(iDatalistCol, "");
						}
					}
					else {
						pDatalistRow->PutValue(iDatalistCol, "");
						pDatalistRow->PutCellBackColor(iDatalistCol, RGB(200,200,200));
					}
				}
			}
			else if(m_nEditingRowSortOrder >= 0)
			{
				if(pRowData->m_nSortOrder == m_nEditingRowSortOrder) {
					// (c.haag 2008-10-27 11:41) - PLID 31834 - If the table is flipped, we
					// actually want the first row of this column to be green
					if (m_bTableRowsAsFields) {
						m_pdlTable->GetFirstRow()->PutCellBackColor(iDatalistCol, RGB(0,255,0));
					} else {
						pDatalistRow->PutCellBackColor(0, RGB(0,255,0));
					}
					pDatalistRow->PutCellBackColor(iDatalistCol, RGB(0,255,0));
					// (z.manning 2011-05-31 09:48) - PLID 42131 - A formula can reference itself if it's for a transformation.
					if(iDetailCol == 0 && IsDlgButtonChecked(IDC_FORMULA_FOR_TRANSFORM) == BST_CHECKED) {
						pDatalistRow->PutValue(iDatalistCol, _bstr_t(GetRowOperand((long)(iDetailRow+1))));
						if(IsDataElementReferencedInElement(pRowData, m_strFormula)) {
							pDatalistRow->PutCellForeColor(iDatalistCol, RGB(0,32,0));
						}
						else {
							pDatalistRow->PutCellForeColor(iDatalistCol, RGB(255,0,0));
						}
					}
					else {
						pDatalistRow->PutValue(iDatalistCol, "");
						pDatalistRow->PutCellForeColor(iDatalistCol, RGB(0,0,0));
					}
				}
				else {
					if( CanItemBeReferencedInFormulaForItem(pRowData, GetCurrentData())) {
						pDatalistRow->PutCellBackColor(iDatalistCol, RGB(255,255,255));
						if(iDetailCol == 0) {
							pDatalistRow->PutValue(iDatalistCol, _bstr_t(GetRowOperand((long)(iDetailRow+1))));
							if(IsDataElementReferencedInElement(pRowData, m_strFormula)) {
								pDatalistRow->PutCellForeColor(iDatalistCol, RGB(0,127,0));
							}
							else {
								pDatalistRow->PutCellForeColor(iDatalistCol, RGB(255,0,0));
							}
						}
						else {
							pDatalistRow->PutValue(iDatalistCol, "");
						}
					}
					else {
						pDatalistRow->PutValue(iDatalistCol, "");
						pDatalistRow->PutCellBackColor(iDatalistCol, RGB(200,200,200));
					}
				}
			}
			else {
				pDatalistRow->PutValue(iDatalistCol, "");
			}
		}
	}

	ReflectCurrentFormula();
	// (j.luckoski 2012-08-31 14:00) - PLID 51755 - Resize height with new column widths
	if(m_bRunResize) {
		ResizeHeight();
	}
	if(bReload) {
		m_richeditFormula.SetFocus();
		m_richeditFormula.SetSel(m_strFormula.GetLength(), m_strFormula.GetLength());
	}
}

void CEmrTableEditCalculatedFieldDlg::SetEditingColumn(long nColumnSortOrder)
{
	m_nEditingColumnSortOrder = nColumnSortOrder;
	m_nEditingRowSortOrder = -1;
}

void CEmrTableEditCalculatedFieldDlg::SetEditingRow(long nRowSortOrder)
{
	m_nEditingRowSortOrder = nRowSortOrder;
	m_nEditingColumnSortOrder = -1;
}

BEGIN_EVENTSINK_MAP(CEmrTableEditCalculatedFieldDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEmrTableEditCalculatedFieldDlg)
	ON_EVENT(CEmrTableEditCalculatedFieldDlg, IDC_SAMPLE_TABLE, 19 /* LeftClick */, OnLeftClickSampleTable, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// (c.haag 2008-10-23 14:52) - PLID 31834 - Renamed nCol to nTableCol to minimize ambiguity with logical detail columns
void CEmrTableEditCalculatedFieldDlg::OnLeftClickSampleTable(LPDISPATCH lpRow, short nTableCol, long x, long y, long nFlags) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		_variant_t varValue = pRow->GetValue(nTableCol);
		if(pRow == NULL || varValue.vt == VT_EMPTY) {
			return;
		}

		BOOL bIsTransform = IsDlgButtonChecked(IDC_FORMULA_FOR_TRANSFORM) == BST_CHECKED;

		CString strCellText = VarString(varValue, "");

		// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method instead of the index property
		if(pRow->CalcRowNumber() == 0 && nTableCol > 0) {
			if(!TestFormula()) {
				return;
			}
			Save();
			// (c.haag 2008-10-23 14:26) - PLID 31834 - Support for table flipping
			CEmrInfoDataElement *pColumnData = (m_bTableRowsAsFields) ? m_aryRows.GetAt(nTableCol - 1) : m_aryColumns.GetAt(nTableCol - 1);
			if(!CanHaveFormula(pColumnData)) {
				return;
			}
			// (c.haag 2008-10-23 14:58) - PLID 31834 - If the table is flipped, we're actually editing a row.
			if (m_bTableRowsAsFields) {
				SetEditingRow(pColumnData->m_nSortOrder);
			} else {
				SetEditingColumn(pColumnData->m_nSortOrder);
			}
			UpdateView(TRUE);
		}
		else if(nTableCol == 0 && pRow->CalcRowNumber() > 0) {
			if(!TestFormula()) {
				return;
			}
			Save();
			// (c.haag 2008-10-23 14:26) - PLID 31834 - Support for table flipping
			CEmrInfoDataElement *pRowData = (m_bTableRowsAsFields) ? m_aryColumns.GetAt(pRow->CalcRowNumber() - 1) : m_aryRows.GetAt(pRow->CalcRowNumber() - 1);
			if(!CanHaveFormula(pRowData)) {
				return;
			}
			// (c.haag 2008-10-23 14:58) - PLID 31834 - If the table is flipped, we're actually editing a column.
			if (m_bTableRowsAsFields) {
				SetEditingColumn(pRowData->m_nSortOrder);
			} else {
				SetEditingRow(pRowData->m_nSortOrder);
			}
			UpdateView(TRUE);
		}
		// (c.haag 2008-10-27 11:56) - PLID 31834 - This conditional is now split out for flipped tables
		else if(!m_bTableRowsAsFields && !strCellText.IsEmpty() && (bIsTransform ||
			((m_nEditingRowSortOrder >= 0 && pRow->CalcRowNumber() != GetCurrentData()->m_nVisibleIndex)
				|| (m_nEditingColumnSortOrder >= 0 && nTableCol != GetCurrentData()->m_nVisibleIndex))))
		{
			InsertFormulaText(strCellText);
		}
		else if(m_bTableRowsAsFields && !strCellText.IsEmpty() && (bIsTransform || 
			((m_nEditingRowSortOrder >= 0 && nTableCol != GetCurrentData()->m_nVisibleIndex)
				|| (m_nEditingColumnSortOrder >= 0 && pRow->CalcRowNumber() != GetCurrentData()->m_nVisibleIndex))))
		{
			InsertFormulaText(strCellText);
		}

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnLeftClickSampleTable");
}

void CEmrTableEditCalculatedFieldDlg::InsertFormulaText(const CString str)
{
	if(!m_richeditFormula.IsWindowEnabled()) {
		return;
	}

	UpdateData(TRUE);
	m_strFormula.Insert(m_nCaretIndex, str);
	ReflectCurrentFormula();
	m_richeditFormula.SetFocus();
	int nNewCursorPos = m_nCaretIndex + str.GetLength();
	m_richeditFormula.SetSel(nNewCursorPos, nNewCursorPos);
	UpdateView(FALSE);
}

void CEmrTableEditCalculatedFieldDlg::OnDivide() 
{
	try
	{
		InsertFormulaText("/");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnDivide");
}

void CEmrTableEditCalculatedFieldDlg::OnExponent() 
{
	try
	{
		InsertFormulaText("^");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnExponent");
}

void CEmrTableEditCalculatedFieldDlg::OnMinus() 
{
	try
	{
		InsertFormulaText("-");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnMinus");
}

void CEmrTableEditCalculatedFieldDlg::OnParentheses() 
{
	try
	{
		InsertFormulaText("()");
		m_richeditFormula.SetSel(m_nCaretIndex - 1, m_nCaretIndex - 1);

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnParentheses");
}

void CEmrTableEditCalculatedFieldDlg::OnPlus() 
{
	try
	{
		InsertFormulaText("+");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnPlus");
}

void CEmrTableEditCalculatedFieldDlg::OnMultiply() 
{
	try
	{
		InsertFormulaText("*");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnMultiply");
}

CEmrInfoDataElementArray* CEmrTableEditCalculatedFieldDlg::GetRows()
{
	return &m_aryRows;
}

CEmrInfoDataElementArray* CEmrTableEditCalculatedFieldDlg::GetColumns()
{
	return &m_aryColumns;
}

void CEmrTableEditCalculatedFieldDlg::Save()
{
	CEmrInfoDataElement *pData = GetCurrentData();

	if(pData == NULL) {
		return;
	}

	GetFormula();
	pData->m_strFormula = m_strFormula;
	pData->m_nDecimalPlaces = m_nDecimalPlaces;
	pData->m_nFlags = 0;
	// (z.manning 2011-05-26 15:33) - PLID 43865 - Handle new options
	if(IsDlgButtonChecked(IDC_FORMULA_SHOW_PLUS_SIGN) == BST_CHECKED) {
		pData->m_nFlags |= edfShowPlusSign;
	}
	if(IsDlgButtonChecked(IDC_FORMULA_FOR_TRANSFORM) == BST_CHECKED) {
		pData->m_nFlags |= edfFormulaForTransform;
	}
}

BOOL CEmrTableEditCalculatedFieldDlg::CanHaveFormula(CEmrInfoDataElement *peide, BOOL bSilent /* = FALSE */)
{
	// (c.haag 2008-10-27 09:19) - PLID 31834 - Proper wording for flipped tables
	CString strKind = (m_bTableRowsAsFields) ? "row" : "column";
	CString strReverseKind = (m_bTableRowsAsFields) ? "column" : "row";

	if(peide->m_nListType >= 3)
	{
		if(peide->m_nListType != LIST_TYPE_TEXT) {
			if(!bSilent) {
				MessageBox("Only text type " + strKind + "s are allowed to have formulas.");
			}
			return FALSE;
		}

		// (r.gonet 08/03/2012) - PLID 51952 - No longer shall we forbid formulas in formulas.
		/*for(long nCol = 0; nCol < m_aryColumns.GetSize(); nCol++)
		{
			CEmrInfoDataElement *peideTemp = m_aryColumns.GetAt(nCol);
			CString strReferencingColumn;
			if(IsDataElementReferencedInAnyFormula(peide, m_aryRows, m_aryColumns, strReferencingColumn))
			{
				if(!bSilent) {
					MessageBox("You may not enter a formula for this " + strKind + " because it is referenced in a formula for " + strKind + " '" + strReferencingColumn + ".'");
				}
				return FALSE;
			}
		}*/
	}
	else
	{
		// (j.jones 2011-07-08 13:16) - PLID 43032 - No formulas in current medication or allergy rows (columns are ok)
		if(m_DataSubType == eistCurrentMedicationsTable) {
			if(!bSilent) {
				CString strWarn;
				if(m_bTableRowsAsFields) {
					strWarn = "You may not enter a formula in a Current Medications table column.";
				}
				else {
					strWarn = "You may not enter a formula in a Current Medications table row.";
				}
				MessageBox(strWarn);
			}
			return FALSE;
		}
		else if(m_DataSubType == eistAllergiesTable) {
			if(!bSilent) {
				CString strWarn;
				if(m_bTableRowsAsFields) {
					strWarn = "You may not enter a formula in an Allergies table column.";
				}
				else {
					strWarn = "You may not enter a formula in an Allergies table row.";
				}
				MessageBox(strWarn);
			}
			return FALSE;
		}

		// (r.gonet 08/03/2012) - PLID 51952 - No longer shall we forbid formulas in formulas.
		/*for(long nRow = 0; nRow < m_aryRows.GetSize(); nRow++)
		{
			CEmrInfoDataElement *peideTemp = m_aryRows.GetAt(nRow);
			CString strReferencingRow;
			if(IsDataElementReferencedInAnyFormula(peide, m_aryRows, m_aryColumns, strReferencingRow))
			{
				if(!bSilent) {
					MessageBox("You may not enter a formula for this " + strReverseKind + " because it is referenced in a formula for " + strReverseKind + " '" + strReferencingRow + ".'");
				}
				return FALSE;
			}
		}*/
	}

	// (z.manning 2010-02-12 13:26) - PLID 37320 - No formulas in smart stamp table built-in columns
	if(IsSmartStampListSubType(peide->m_nListSubType)) {
		if(!bSilent) {
			MessageBox("You may not enter a formula in a built-in Smart Stamp table item.");
		}
		return FALSE;
	}

	// (j.jones 2011-05-03 16:08) - PLID 43527 - No formulas in current medication table built-in columns
	if(IsCurrentMedicationListSubType(peide->m_nListSubType)) {
		if(!bSilent) {
			MessageBox("You may not enter a formula in a built-in Current Medications table item.");
		}
		return FALSE;
	}

	// (z.manning 2010-04-20 11:33) - PLID 29301 - Labels cannot have formulas
	if(peide->m_bIsLabel) {
		if(!bSilent) {
			MessageBox("This item is a label and may not have a formula.");
		}
		return FALSE;
	}

	return TRUE;
}

CEmrInfoDataElement* CEmrTableEditCalculatedFieldDlg::GetCurrentData()
{
	if(m_nEditingColumnSortOrder >= 0) {
		for(int nCol = 0; nCol < m_aryColumns.GetSize(); nCol++) {
			CEmrInfoDataElement *pColData = m_aryColumns.GetAt(nCol);
			if(pColData->m_nSortOrder == m_nEditingColumnSortOrder) {
				return pColData;
			}
		}
	}
	else if(m_nEditingRowSortOrder >= 0) {
		for(int nRow = 0; nRow < m_aryRows.GetSize(); nRow++) {
			CEmrInfoDataElement *pRowData = m_aryRows.GetAt(nRow);
			if(pRowData->m_nSortOrder == m_nEditingRowSortOrder) {
				return pRowData;
			}
		}
	}

	return NULL;
}

void CEmrTableEditCalculatedFieldDlg::OnOK() 
{
	try
	{
		// (z.manning 2008-08-15 11:08) - Make sure our data is valid (specifically decimal places)
		UINT nDecimalPlaces = GetDlgItemInt(IDC_DECIMAL_PLACES, NULL, FALSE);
		if(nDecimalPlaces < 0 || nDecimalPlaces > 255) {
			MessageBox("Please enter a value between 0 and 255 for decimal places.");
			return;
		}

		if(!TestFormula()) {
			return;
		}

		Save();

		CNxDialog::OnOK();

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnOK");
}

void CEmrTableEditCalculatedFieldDlg::OnClearFormula() 
{
	try
	{
		m_strFormula.Empty();
		ReflectCurrentFormula();
		UpdateView(FALSE);

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnClearFormula");
}

void CEmrTableEditCalculatedFieldDlg::OnDot() 
{
	try
	{
		InsertFormulaText(".");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnDot");
}

void CEmrTableEditCalculatedFieldDlg::OnNumber0() 
{
	try
	{
		InsertFormulaText("0");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnNumber0");
}

void CEmrTableEditCalculatedFieldDlg::OnNumber1() 
{
	try
	{
		InsertFormulaText("1");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnNumber1");
}

void CEmrTableEditCalculatedFieldDlg::OnNumber2() 
{
	try
	{
		InsertFormulaText("2");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnNumber2");
}

void CEmrTableEditCalculatedFieldDlg::OnNumber3() 
{
	try
	{
		InsertFormulaText("3");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnNumber3");
}

void CEmrTableEditCalculatedFieldDlg::OnNumber4() 
{
	try
	{
		InsertFormulaText("4");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnNumber4");
}

void CEmrTableEditCalculatedFieldDlg::OnNumber5() 
{
	try
	{
		InsertFormulaText("5");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnNumber5");
}

void CEmrTableEditCalculatedFieldDlg::OnNumber6() 
{
	try
	{
		InsertFormulaText("6");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnNumber6");
}

void CEmrTableEditCalculatedFieldDlg::OnNumber7() 
{
	try
	{
		InsertFormulaText("7");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnNumber7");
}

void CEmrTableEditCalculatedFieldDlg::OnNumber8() 
{
	try
	{
		InsertFormulaText("8");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnNumber8");
}

void CEmrTableEditCalculatedFieldDlg::OnNumber9() 
{
	try
	{
		InsertFormulaText("9");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnNumber9");
}

// (z.manning 2011-05-26 10:18) - PLID 43851
void CEmrTableEditCalculatedFieldDlg::OnBnClickedModulo()
{
	try
	{
		InsertFormulaText("%");

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnNumber9");
}

void CEmrTableEditCalculatedFieldDlg::OnSelchangeFormula(NMHDR* pNMHDR, LRESULT* pResult) 
{
	try
	{
		SELCHANGE *pSelChange = reinterpret_cast<SELCHANGE *>(pNMHDR);
		
		// (z.manning 2008-06-10 09:51) - Remember the current caret position.
		m_nCaretIndex = pSelChange->chrg.cpMax;
		
		*pResult = 0;

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnSelchangeFormula");
}

void CEmrTableEditCalculatedFieldDlg::OnBackspace() 
{
	try
	{
		if(m_nCaretIndex > 0) {
			UpdateData(TRUE);
			m_nCaretIndex--;
			m_strFormula.Delete(m_nCaretIndex);
			ReflectCurrentFormula();
		}
		m_richeditFormula.SetFocus();
		m_richeditFormula.SetSel(m_nCaretIndex, m_nCaretIndex);
		UpdateView(FALSE);

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnBackspace");
}

// (z.manning 2008-06-09 11:05) - PLID 30145 - Returns true if the item being referenced is allowed to be
// in the formula item's formula.
BOOL CEmrTableEditCalculatedFieldDlg::CanItemBeReferencedInFormulaForItem(IN CEmrInfoDataElement *peideReference, IN CEmrInfoDataElement *peideFormula)
{
	if(peideReference->m_nListType == LIST_TYPE_LINKED || peideReference->m_nListType == LIST_TYPE_CHECKBOX) {
		return FALSE;
	}

	// (z.manning 2011-05-31 09:35) - PLID 42131 - Other formula rows can be referenced if this is a transform formula.
	BOOL bIsFormulaTransform;
	if(peideFormula == GetCurrentData()) {
		bIsFormulaTransform = IsDlgButtonChecked(IDC_FORMULA_FOR_TRANSFORM) == BST_CHECKED;
	}
	else {
		bIsFormulaTransform = (peideFormula->m_nFlags & edfFormulaForTransform) != 0;
	}
	BOOL bIsReferenceTransform;
	if(peideReference == GetCurrentData()) {
		bIsReferenceTransform = IsDlgButtonChecked(IDC_FORMULA_FOR_TRANSFORM) == BST_CHECKED;
	}
	else {
		bIsReferenceTransform = (peideReference->m_nFlags & edfFormulaForTransform) != 0;
	}

	// (z.manning 2010-05-04 10:35) - PLID 29301 - Labels can't be in formulas
	if(peideReference->m_bIsLabel) {
		return FALSE;
	}

	return TRUE;
}

void CEmrTableEditCalculatedFieldDlg::SetReadOnly(BOOL bReadOnly)
{
	m_richeditFormula.EnableWindow(!bReadOnly);
	m_nxeditDecimalPlaces.EnableWindow(!bReadOnly);
	m_btnTestFormula.EnableWindow(!bReadOnly);
	m_btnClearFormula.EnableWindow(!bReadOnly);
	GetDlgItem(IDC_FORMULA_SHOW_PLUS_SIGN)->EnableWindow(!bReadOnly);
	GetDlgItem(IDC_FORMULA_FOR_TRANSFORM)->EnableWindow(!bReadOnly);
}

void CEmrTableEditCalculatedFieldDlg::OnChangeFormula() 
{
	try
	{
		// (z.manning 2008-06-23 10:04) - We need to referesh anything that may need updated after
		// changing text such as the cell text colors.
		UpdateView(FALSE);

	}NxCatchAll("CEmrTableEditCalculatedFieldDlg::OnChangeFormula");
}

void CEmrTableEditCalculatedFieldDlg::ReflectCurrentFormula()
{
	UpdateData(FALSE);

	if(GetCurrentData() == NULL) {
		return;
	}

	long nIndex = GetCurrentData()->m_nVisibleIndex;
	IRowSettingsPtr pRow = m_pdlTable->GetFirstRow();

	// (c.haag 2008-10-27 11:29) - PLID 31834 - Table flipping
	int nEditingColumnSortOrder = (m_bTableRowsAsFields) ? m_nEditingRowSortOrder : m_nEditingColumnSortOrder;
	int nEditingRowSortOrder = (m_bTableRowsAsFields) ? m_nEditingColumnSortOrder : m_nEditingRowSortOrder;

	if(nEditingColumnSortOrder != -1) {
		// (z.manning 2008-06-25 15:09) - Get the first non-tile row
		pRow = pRow->GetNextRow();
		// (z.manning 2011-05-31 09:14) - PLID 42131 - Don't show the formula in the datalist if it's a transform formula
		if(pRow != NULL && IsDlgButtonChecked(IDC_FORMULA_FOR_TRANSFORM) == BST_UNCHECKED) {
			pRow->PutValue((short)nIndex, _bstr_t(m_strFormula));
		}
		IColumnSettingsPtr pCol = m_pdlTable->GetColumn((short)nIndex);
		if(pCol != NULL) {
			long nIdealColumnWidth = pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE);
			pCol->PutStoredWidth(nIdealColumnWidth);
		}
	}
	else if(nEditingRowSortOrder != -1) {
		IColumnSettingsPtr pCol = m_pdlTable->GetColumn(1);
		if(pCol != NULL) {
			for(long nCount = 0; nCount < nIndex; nCount++) {
				pRow = pRow->GetNextRow();
			}
			// (z.manning 2011-05-31 09:14) - PLID 42131 - Don't show the formula in the datalist if it's a transform formula
			if(IsDlgButtonChecked(IDC_FORMULA_FOR_TRANSFORM) == BST_UNCHECKED) {
				pRow->PutValue(1, _bstr_t(m_strFormula));
			}

			long nIdealColumnWidth = pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE);
			pCol->PutStoredWidth(nIdealColumnWidth);
		}
	}
}

// (z.manning 2011-05-31 09:40) - PLID 42131
void CEmrTableEditCalculatedFieldDlg::OnBnClickedFormulaForTransform()
{
	try
	{
		UpdateView(FALSE);
	}
	NxCatchAll(__FUNCTION__);
}

// (j.luckoski 2012-08-31 14:10) - PLID 51755 - ResizeHeight based on column widths and if scrollbar is visible
void CEmrTableEditCalculatedFieldDlg::ResizeHeight()
{
	// (j.luckoski 2012-08-30 14:18) - PLID 51755 - Handle the width so we can add more to the height if the scrollbar is appearing.
		long nTempIdealTableWidth = 0;
		int nTableColumns = (m_bTableRowsAsFields) ? m_aryRows.GetSize() : m_aryColumns.GetSize();

		// (c.haag 2008-10-23 12:42) - PLID 31834 - Table flipping support
		for(long nColIndex = 0; nColIndex < nTableColumns; nColIndex++)
		{
			IColumnSettingsPtr pCol = m_pdlTable->GetColumn((short)nColIndex + 1);
			if(pCol != NULL) {
				long nIdealColumnWidth = pCol->CalcWidthFromData(VARIANT_TRUE, VARIANT_FALSE);
				pCol->PutStoredWidth(nIdealColumnWidth);
				// (j.luckoski 2012-08-30 14:18) - PLID 51755 - Add the width to the table width
				nTempIdealTableWidth += nIdealColumnWidth;
			}
		}

		CRect rcDialog, rcDatalist;
		GetWindowRect(rcDialog);
		ScreenToClient(rcDialog);
		CWnd *pwndDatalist = GetDlgItem(IDC_SAMPLE_TABLE);
		pwndDatalist->GetWindowRect(rcDatalist);
		ScreenToClient(rcDatalist);

		if(m_nIdealTableWidth < (rcDatalist.Width() - 49) && nTempIdealTableWidth >= (rcDatalist.Width() - 49)) {
			pwndDatalist->MoveWindow(rcDatalist.left, rcDatalist.top, rcDatalist.Width(), rcDatalist.Height() + 19);
			MoveWindow(rcDialog.left, rcDialog.top, rcDialog.Width(), rcDialog.Height() + 19);
			CenterWindow();
			m_nIdealTableWidth = nTempIdealTableWidth;
		} else if(m_nIdealTableWidth >= (rcDatalist.Width() - 49) && nTempIdealTableWidth < (rcDatalist.Width() - 49)) {
			pwndDatalist->MoveWindow(rcDatalist.left, rcDatalist.top, rcDatalist.Width(), rcDatalist.Height() - 19);
			MoveWindow(rcDialog.left, rcDialog.top, rcDialog.Width(), rcDialog.Height() - 19);
			CenterWindow();
			m_nIdealTableWidth = nTempIdealTableWidth;
		}	
}