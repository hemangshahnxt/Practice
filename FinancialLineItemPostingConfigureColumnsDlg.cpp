// CFinancialLineItemPostingConfigureColumnsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "FinancialLineItemPostingConfigureColumnsDlg.h"
#include "GlobalFinancialUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;


// (z.manning 2008-06-18 17:27) - PLID 26544 - Created
/////////////////////////////////////////////////////////////////////////////
// CFinancialLineItemPostingConfigureColumnsDlg dialog


CFinancialLineItemPostingConfigureColumnsDlg::CFinancialLineItemPostingConfigureColumnsDlg(CFinancialLineItemPostingDlg* pParent)
	: CNxDialog(CFinancialLineItemPostingConfigureColumnsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CFinancialLineItemPostingConfigureColumnsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	m_pdlgLineItemPosting = pParent;
}


void CFinancialLineItemPostingConfigureColumnsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFinancialLineItemPostingConfigureColumnsDlg)
	DDX_Control(pDX, IDC_LINE_ITEM_COLUMN_DOWN, m_btnDown);
	DDX_Control(pDX, IDC_LINE_ITEM_COLUMN_UP, m_btnUp);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDOK, m_btnOk);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFinancialLineItemPostingConfigureColumnsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CFinancialLineItemPostingConfigureColumnsDlg)
	ON_BN_CLICKED(IDC_LINE_ITEM_COLUMN_DOWN, OnLineItemColumnDown)
	ON_BN_CLICKED(IDC_LINE_ITEM_COLUMN_UP, OnLineItemColumnUp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFinancialLineItemPostingConfigureColumnsDlg message handlers

BOOL CFinancialLineItemPostingConfigureColumnsDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnCancel.AutoSet(NXB_CANCEL);
		m_btnOk.AutoSet(NXB_OK);
		m_btnUp.AutoSet(NXB_UP);
		m_btnDown.AutoSet(NXB_DOWN);

		m_pdlColumns = BindNxDataList2Ctrl(IDC_LINE_ITEM_COLUMNS, false);

		// (z.manning 2008-06-19 14:04) - PLID 26544 - Load all the names of the dynamic columns.
		for(short nCol = CFinancialLineItemPostingDlg::plcBeginDynamicCols; nCol < m_pdlgLineItemPosting->m_PostingList->GetColumnCount(); nCol++)
		{
			IRowSettingsPtr pRow = m_pdlColumns->GetNewRow();
			CString strColumnName = (LPCTSTR)m_pdlgLineItemPosting->m_PostingList->GetColumn(nCol)->GetColumnTitle();
			pRow->PutValue(clcColumnName, _bstr_t(strColumnName));

			// (j.jones 2014-06-27 14:44) - PLID 62631 - vision payments don't show the deductible, coinsurance,
			// or allowable columns, so gray them out instead of hiding such that you can still move them around
			if (m_pdlgLineItemPosting->GetPayType() == eVisionPayment &&
				(strColumnName == "Deductible" || strColumnName == "CoIns." || strColumnName == "Allowable")) {

				pRow->PutForeColor(RGB(128, 128, 128));
			}

			m_pdlColumns->AddRowAtEnd(pRow, NULL);
		}

		UpdateButtons();

	}NxCatchAll("CFinancialLineItemPostingConfigureColumnsDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFinancialLineItemPostingConfigureColumnsDlg::OnLineItemColumnDown() 
{
	try
	{
		IRowSettingsPtr pRow1 = m_pdlColumns->GetCurSel();
		IRowSettingsPtr pRow2 = pRow1->GetNextRow();
		SwapColumnRows(pRow1, pRow2);
		m_pdlColumns->PutCurSel(pRow2);
		UpdateButtons();

	}NxCatchAll("CFinancialLineItemPostingConfigureColumnsDlg::OnLineItemColumnDown");
}

void CFinancialLineItemPostingConfigureColumnsDlg::OnLineItemColumnUp() 
{
	try
	{
		IRowSettingsPtr pRow1 = m_pdlColumns->GetCurSel();
		IRowSettingsPtr pRow2 = pRow1->GetPreviousRow();
		SwapColumnRows(pRow1, pRow2);
		m_pdlColumns->PutCurSel(pRow2);
		UpdateButtons();

	}NxCatchAll("CFinancialLineItemPostingConfigureColumnsDlg::OnLineItemColumnUp");
}

void CFinancialLineItemPostingConfigureColumnsDlg::OnOK() 
{
	try
	{
		// (z.manning 2008-06-19 11:09) - PLID 26544 - Store a semicolon delimited list of the column
		// names in order.
		CString strColumnList;
		for(IRowSettingsPtr pRow = m_pdlColumns->GetFirstRow(); pRow != NULL; pRow = pRow->GetNextRow()) {
			strColumnList += VarString(pRow->GetValue(clcColumnName)) + ';';
		}
		SetRemotePropertyText("LineItemPostingColumnList", strColumnList, 0, "<None>");
	
		CDialog::OnOK();

	}NxCatchAll("CFinancialLineItemPostingConfigureColumnsDlg::OnOK");
}

void CFinancialLineItemPostingConfigureColumnsDlg::UpdateButtons()
{
	IRowSettingsPtr pRow = m_pdlColumns->GetCurSel();
	if(pRow == NULL) {
		m_btnUp.EnableWindow(FALSE);
		m_btnDown.EnableWindow(FALSE);
		return;
	}

	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method instead of the index
	if(pRow->CalcRowNumber() > 0) {
		m_btnUp.EnableWindow(TRUE);
	}
	else {
		m_btnUp.EnableWindow(FALSE);
	}

	// (b.cardillo 2008-06-27 16:17) - PLID 30529 - Changed to use the new index-less method instead of the index
	if(pRow->CalcRowNumber() < m_pdlColumns->GetRowCount() - 1) {
		m_btnDown.EnableWindow(TRUE);
	}
	else {
		m_btnDown.EnableWindow(FALSE);
	}
}

BEGIN_EVENTSINK_MAP(CFinancialLineItemPostingConfigureColumnsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CFinancialLineItemPostingConfigureColumnsDlg)
	ON_EVENT(CFinancialLineItemPostingConfigureColumnsDlg, IDC_LINE_ITEM_COLUMNS, 2 /* SelChanged */, OnSelChangedLineItemColumns, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CFinancialLineItemPostingConfigureColumnsDlg::OnSelChangedLineItemColumns(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try
	{
		UpdateButtons();

	}NxCatchAll("CFinancialLineItemPostingConfigureColumnsDlg::OnSelChangedLineItemColumns");
}

void CFinancialLineItemPostingConfigureColumnsDlg::SwapColumnRows(IRowSettingsPtr pRow1, IRowSettingsPtr pRow2)
{
	if(pRow1 == NULL || pRow2 == NULL) {
		// (z.manning 2008-06-19 11:03) - This function shouldn't have been called.
		ASSERT(FALSE);
		return;
	}

	_variant_t varTemp = pRow1->GetValue(clcColumnName);
	pRow1->PutValue(clcColumnName, pRow2->GetValue(clcColumnName));
	pRow2->PutValue(clcColumnName, varTemp);
}