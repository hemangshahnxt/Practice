// SelectStampDlg.cpp : implementation file
//

#include "stdafx.h"
#include "EmrRc.h"
#include "SelectStampDlg.h"

//TES 2/24/2012 - PLID 45127 - Created
// CSelectStampDlg dialog

IMPLEMENT_DYNAMIC(CSelectStampDlg, CNxDialog)

CSelectStampDlg::CSelectStampDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSelectStampDlg::IDD, pParent)
{
	m_nSelectedStampID = -1;
	m_clrSelectedColor = RGB(0,0,0);
	m_bSelectedShowDot = FALSE;
	m_nImageEmrInfoMasterID = -1;
	//TES 3/28/2012 - PLID 49294 - Default to filtering our list of stamps
	m_bFilteringStamps = true;
}

CSelectStampDlg::~CSelectStampDlg()
{
}

void CSelectStampDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSelectStampDlg)
	DDX_Control(pDX, IDOK, m_nxbOK);
	DDX_Control(pDX, IDCANCEL, m_nxbCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSelectStampDlg, CNxDialog)
END_MESSAGE_MAP()


// CSelectStampDlg message handlers
using namespace NXDATALIST2Lib;
enum StampColumns {
	scID = 0,
	scStampText = 1,
	scTypeName = 2,
	scColor = 3,
	scShowDot = 4,
};

BOOL CSelectStampDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		//TES 2/24/2012 - PLID 45127 - NxIconify
		m_nxbOK.AutoSet(NXB_OK);
		m_nxbCancel.AutoSet(NXB_CANCEL);

		//TES 2/24/2012 - PLID 45127 - Load our list of stamps
		m_pStampList = BindNxDataList2Ctrl(IDC_STAMP_LIST, false);
		//TES 3/28/2012 - PLID 49294 - Moved the requerying into its own function
		RequeryStampList();


	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSelectStampDlg::OnOK()
{
	try {

		//TES 2/24/2012 - PLID 45127 - Get the stamp they selected.
		IRowSettingsPtr pRow = m_pStampList->CurSel;
		//TES 3/28/2012 - PLID 49294 - Don't let them select the <Show All Stamps> row
		// (a.walling 2012-03-29 08:35) - PLID 49294 - A misplaced parentheses was causing trouble
		if(pRow == NULL || -1 == VarLong(pRow->GetValue(scID))) {
			MsgBox("Please select a stamp from the list");
			return;
		}
		m_nSelectedStampID = VarLong(pRow->GetValue(scID));
		m_strSelectedStampText = VarString(pRow->GetValue(scStampText));
		m_strSelectedTypeName = VarString(pRow->GetValue(scTypeName));
		m_clrSelectedColor = (COLORREF)VarLong(pRow->GetValue(scColor));
		m_bSelectedShowDot = VarBool(pRow->GetValue(scShowDot));


		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CSelectStampDlg::GetSelectedStamp(long &nStampID, CString &strStampText, CString &strTypeName, COLORREF &clrStampColor)
{
	nStampID = m_nSelectedStampID;
	strStampText = m_strSelectedStampText;
	if(m_bSelectedShowDot) {
		strStampText = CString((char)149) + _T(" ") + strStampText;
	}
	strTypeName = m_strSelectedTypeName;
	clrStampColor = m_clrSelectedColor;
}

BEGIN_EVENTSINK_MAP(CSelectStampDlg, CNxDialog)
ON_EVENT(CSelectStampDlg, IDC_STAMP_LIST, 3, CSelectStampDlg::OnDblClickCellStampList, VTS_DISPATCH VTS_I2)
ON_EVENT(CSelectStampDlg, IDC_STAMP_LIST, 18, CSelectStampDlg::OnRequeryFinishedStampList, VTS_I2)
END_EVENTSINK_MAP()

void CSelectStampDlg::OnDblClickCellStampList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		//TES 3/6/2012 - PLID 45127 - Save a little mouse movement by allowing them to dismiss the dialog by double clicking.
		IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL) {
			long nID = VarLong(pRow->GetValue(scID));
			//TES 3/28/2012 - PLID 49294 - If they double-click on the <Show All Stamps>/<Filter Stamp List> row (which has an ID of -1), then
			// toggle the filtering and requery, otherwise select the row they double-clicked on and go straight to OnOK().
			if(nID == -1) {
				m_bFilteringStamps = !m_bFilteringStamps;
				RequeryStampList();
			}
			else {
				m_pStampList->CurSel = pRow;
				OnOK();
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void CSelectStampDlg::RequeryStampList()
{
	CString strWhereClause;

	//TES 3/28/2012 - PLID 49294 - Change the query based on whether or not we're filtering the list.
	if(m_bFilteringStamps) {
		strWhereClause.Format("Inactive = 0 AND ID NOT IN (SELECT StampID FROM EmrInfoStampExclusionsT WHERE EmrInfoMasterID = %li)", 
			m_nImageEmrInfoMasterID);
	}
	else {
		strWhereClause = "Inactive = 0";
	}
	m_pStampList->WhereClause = _bstr_t(strWhereClause);
	m_pStampList->Requery();
}

void CSelectStampDlg::OnRequeryFinishedStampList(short nFlags)
{
	try {
		//TES 3/28/2012 - PLID 49294 - Add a row at the end giving them the option to toggle whether or not they're filtering the stamp list
		IRowSettingsPtr pRow = m_pStampList->GetNewRow();
		pRow->PutValue(scID, (long)-1);
		CString strText = m_bFilteringStamps?" <Show All Stamps> " : " <Filter Stamp List> ";
		pRow->PutValue(scStampText, _bstr_t(strText));
		pRow->PutValue(scTypeName, _bstr_t(""));
		pRow->PutValue(scColor, (long)0);
		pRow->PutValue(scShowDot, g_cvarFalse);
		m_pStampList->AddRowAtEnd(pRow, NULL);
	}NxCatchAll(__FUNCTION__);
}
