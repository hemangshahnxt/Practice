// CLIAServicesSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CLIAServicesSetupDlg.h"

// (j.jones 2011-10-19 09:37) - PLID 46023 - created

// CCLIAServicesSetupDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

IMPLEMENT_DYNAMIC(CCLIAServicesSetupDlg, CNxDialog)

enum ServiceColumns {

	scID = 0,
	scCode,
	scSubCode,
	scDescription,
};

CCLIAServicesSetupDlg::CCLIAServicesSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CCLIAServicesSetupDlg::IDD, pParent)
{
	m_nBkgColor = GetNxColor(GNC_ADMIN, 0);
}

CCLIAServicesSetupDlg::~CCLIAServicesSetupDlg()
{
}

void CCLIAServicesSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_CPT, m_btnUnselectOneCPT);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_CPT, m_btnUnselectAllCPT);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_CPT, m_btnSelectOneCPT);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL_CPT, m_btnSelectAllCPT);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_BKG_CLIA_CODES, m_bkg1);
}


BEGIN_MESSAGE_MAP(CCLIAServicesSetupDlg, CNxDialog)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_CPT, OnBtnSelectOneCpt)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL_CPT, OnBtnSelectAllCpt)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_CPT, OnBtnUnselectOneCpt)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_CPT, OnBtnUnselectAllCpt)
	ON_BN_CLICKED(IDOK, OnOk)
	ON_BN_CLICKED(IDCANCEL, OnCancel)
END_MESSAGE_MAP()


// CCLIAServicesSetupDlg message handlers

BOOL CCLIAServicesSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		m_UnselectedCPTList = BindNxDataList2Ctrl(IDC_UNSELECTED_CLIA_SERVICE_LIST);
		m_SelectedCPTList = BindNxDataList2Ctrl(IDC_SELECTED_CLIA_SERVICE_LIST);

		m_btnSelectOneCPT.AutoSet(NXB_RIGHT);
		m_btnSelectAllCPT.AutoSet(NXB_RRIGHT);
		m_btnUnselectOneCPT.AutoSet(NXB_LEFT);
		m_btnUnselectAllCPT.AutoSet(NXB_LLEFT);

		m_bkg1.SetColor(m_nBkgColor);
		
		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

	}NxCatchAll("CCLIAServicesSetupDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CCLIAServicesSetupDlg, CNxDialog)
	ON_EVENT(CCLIAServicesSetupDlg, IDC_UNSELECTED_CLIA_SERVICE_LIST, 3, OnDblClickCellUnselectedCliaServiceList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CCLIAServicesSetupDlg, IDC_SELECTED_CLIA_SERVICE_LIST, 3, OnDblClickCellSelectedCliaServiceList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CCLIAServicesSetupDlg::OnDblClickCellUnselectedCliaServiceList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);

		m_UnselectedCPTList->CurSel = pRow;

		if(pRow) {
			OnBtnSelectOneCpt();
		}

	}NxCatchAll(__FUNCTION__);
}

void CCLIAServicesSetupDlg::OnDblClickCellSelectedCliaServiceList(LPDISPATCH lpRow, short nColIndex)
{
	try {

		IRowSettingsPtr pRow(lpRow);

		m_SelectedCPTList->CurSel = pRow;

		if(pRow) {
			OnBtnUnselectOneCpt();
		}

	}NxCatchAll(__FUNCTION__);
}

void CCLIAServicesSetupDlg::OnOk()
{
	try {

		//save selected CPT codes

		CString strSqlBatch;

		AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET UseCLIA = 0 WHERE UseCLIA <> 0");

		IRowSettingsPtr pRow = m_SelectedCPTList->GetFirstRow();
		while(pRow) {
			long ServiceID = VarLong(pRow->GetValue(scID));
			//cannot be parameterized, because we could exceed the parameter limit
			AddStatementToSqlBatch(strSqlBatch, "UPDATE ServiceT SET UseCLIA = 1 WHERE ID = %li", ServiceID);

			pRow = pRow->GetNextRow();
		}

		ExecuteSqlBatch(strSqlBatch);

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

void CCLIAServicesSetupDlg::OnCancel()
{
	try {

		CNxDialog::OnCancel();

	}NxCatchAll(__FUNCTION__);
}

void CCLIAServicesSetupDlg::OnBtnSelectOneCpt() 
{
	try {

		m_SelectedCPTList->TakeCurrentRowAddSorted(m_UnselectedCPTList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CCLIAServicesSetupDlg::OnBtnSelectAllCpt() 
{
	try {

		m_SelectedCPTList->TakeAllRows(m_UnselectedCPTList);

	}NxCatchAll(__FUNCTION__);
}

void CCLIAServicesSetupDlg::OnBtnUnselectOneCpt() 
{
	try {

		m_UnselectedCPTList->TakeCurrentRowAddSorted(m_SelectedCPTList, NULL);

	}NxCatchAll(__FUNCTION__);
}

void CCLIAServicesSetupDlg::OnBtnUnselectAllCpt() 
{
	try {

		m_UnselectedCPTList->TakeAllRows(m_SelectedCPTList);

	}NxCatchAll(__FUNCTION__);
}