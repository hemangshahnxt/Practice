// TelevoxConfigureFieldsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TelevoxConfigureFieldsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALIST2Lib;

// (z.manning 2008-07-10 16:41) - PLID 20543 - Created
/////////////////////////////////////////////////////////////////////////////
// CTelevoxConfigureFieldsDlg dialog


CTelevoxConfigureFieldsDlg::CTelevoxConfigureFieldsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CTelevoxConfigureFieldsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTelevoxConfigureFieldsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CTelevoxConfigureFieldsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTelevoxConfigureFieldsDlg)
	DDX_Control(pDX, IDC_TELEVOX_FIELD_UP, m_btnUp);
	DDX_Control(pDX, IDC_TELEVOX_FIELD_DOWN, m_btnDown);
	DDX_Control(pDX, IDCANCEL, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTelevoxConfigureFieldsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTelevoxConfigureFieldsDlg)
	ON_BN_CLICKED(IDC_TELEVOX_FIELD_UP, OnTelevoxFieldUp)
	ON_BN_CLICKED(IDC_TELEVOX_FIELD_DOWN, OnTelevoxFieldDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTelevoxConfigureFieldsDlg message handlers

BEGIN_EVENTSINK_MAP(CTelevoxConfigureFieldsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CTelevoxConfigureFieldsDlg)
	ON_EVENT(CTelevoxConfigureFieldsDlg, IDC_TELEVOX_FIELD_LIST, 10 /* EditingFinished */, OnEditingFinishedTelevoxFieldList, VTS_DISPATCH VTS_I2 VTS_VARIANT VTS_VARIANT VTS_BOOL)
	ON_EVENT(CTelevoxConfigureFieldsDlg, IDC_TELEVOX_FIELD_LIST, 2 /* SelChanged */, OnSelChangedTelevoxFieldList, VTS_DISPATCH VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CTelevoxConfigureFieldsDlg::OnInitDialog() 
{
	try
	{
		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnUp.AutoSet(NXB_UP);
		m_btnDown.AutoSet(NXB_DOWN);

		m_pdlFields = BindNxDataList2Ctrl(IDC_TELEVOX_FIELD_LIST, GetRemoteData(), true);

		UpdateButtons();

	}NxCatchAll("CTelevoxConfigureFieldsDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTelevoxConfigureFieldsDlg::OnTelevoxFieldUp() 
{
	try
	{
		IRowSettingsPtr pRow = m_pdlFields->GetCurSel();
		SwapRows(pRow, pRow->GetPreviousRow());

	}NxCatchAll("CTelevoxConfigureFieldsDlg::OnTelevoxFieldUp");
}

void CTelevoxConfigureFieldsDlg::OnTelevoxFieldDown() 
{
	try
	{
		IRowSettingsPtr pRow = m_pdlFields->GetCurSel();
		SwapRows(pRow, pRow->GetNextRow());

	}NxCatchAll("CTelevoxConfigureFieldsDlg::OnTelevoxFieldDown");
}

void CTelevoxConfigureFieldsDlg::OnEditingFinishedTelevoxFieldList(LPDISPATCH lpRow, short nCol, const VARIANT FAR& varOldValue, const VARIANT FAR& varNewValue, BOOL bCommit) 
{
	try
	{
		IRowSettingsPtr pRow(lpRow);
		if(pRow == NULL) {
			return;
		}

		long nFieldID = VarLong(pRow->GetValue(flcID));
		switch(nCol)
		{
			case flcCheck:
				ASSERT(varNewValue.vt == VT_BOOL);
				// (z.manning 2008-07-10 16:43) - PLID 20543 - Save the export status of this field
				ExecuteParamSql("UPDATE TelevoxFieldsT SET IsExported = {VT_BOOL} WHERE ID = {INT}", varNewValue, nFieldID);
				break;
		}

	}NxCatchAll("CTelevoxConfigureFieldsDlg::OnEditingFinishedTelevoxFieldList");
}

void CTelevoxConfigureFieldsDlg::UpdateButtons()
{
	IRowSettingsPtr pRow = m_pdlFields->GetCurSel();
	if(pRow == NULL) {
		m_btnUp.EnableWindow(FALSE);
		m_btnDown.EnableWindow(FALSE);
		return;
	}

	m_btnDown.EnableWindow(pRow->GetNextRow() != NULL);
	m_btnUp.EnableWindow(pRow->GetPreviousRow() != NULL);
}

void CTelevoxConfigureFieldsDlg::OnSelChangedTelevoxFieldList(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try
	{
		UpdateButtons();

	}NxCatchAll("CTelevoxConfigureFieldsDlg::OnSelChangedTelevoxFieldList");
}

void CTelevoxConfigureFieldsDlg::SwapRows(IRowSettingsPtr pRow1, IRowSettingsPtr pRow2)
{
	if(pRow1 == NULL || pRow2 == NULL) {
		// (z.manning 2008-07-10 13:13) - Shouldn't have called this
		ASSERT(FALSE);
		return;
	}

	long nID1 = VarLong(pRow1->GetValue(flcID));
	BYTE nSortOrder1 = VarByte(pRow1->GetValue(flcSortOrder));

	long nID2 = VarLong(pRow2->GetValue(flcID));
	BYTE nSortOrder2 = VarByte(pRow2->GetValue(flcSortOrder));

	// (z.manning 2008-07-10 13:27) - Swap sort orders (note: we first temporarily set
	// one of them to 0 to avoid violating the unique constraint of sort order.
	ExecuteParamSql(
		"UPDATE TelevoxFieldsT SET SortOrder = 0 WHERE ID = {INT} \r\n"
		"UPDATE TelevoxFieldsT SET SortOrder = {INT} WHERE ID = {INT} \r\n"
		"UPDATE TelevoxFieldsT SET SortOrder = {INT} WHERE ID = {INT} \r\n"
		, nID1, nSortOrder1, nID2, nSortOrder2, nID1);

	// (z.manning 2008-07-10 15:13) - Now swap the rows in the list
	_variant_t varID, varCheck, varDisplayName;
	varID = pRow1->GetValue(flcID);
	varCheck = pRow1->GetValue(flcCheck);
	varDisplayName = pRow1->GetValue(flcDisplayName);

	pRow1->PutValue(flcID, pRow2->GetValue(flcID));
	pRow1->PutValue(flcCheck, pRow2->GetValue(flcCheck));
	pRow1->PutValue(flcDisplayName, pRow2->GetValue(flcDisplayName));

	pRow2->PutValue(flcID, varID);
	pRow2->PutValue(flcCheck, varCheck);
	pRow2->PutValue(flcDisplayName, varDisplayName);

	m_pdlFields->PutCurSel(pRow2);
	UpdateButtons();
}
