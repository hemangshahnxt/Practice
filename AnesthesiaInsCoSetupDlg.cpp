// AnesthesiaInsCoSetupDlg.cpp : implementation file
//

#include "stdafx.h"
#include "AnesthesiaInsCoSetupDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAnesthesiaInsCoSetupDlg dialog


CAnesthesiaInsCoSetupDlg::CAnesthesiaInsCoSetupDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAnesthesiaInsCoSetupDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnesthesiaInsCoSetupDlg)
	//}}AFX_DATA_INIT
}


void CAnesthesiaInsCoSetupDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnesthesiaInsCoSetupDlg)
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_ANESTH_INSCO, m_btnUnselectOneInsCo);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_ANESTH_INSCO, m_btnUnselectAllInsCo);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL_ANESTH_INSCO, m_btnSelectAllInsCo);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_ANESTH_INSCO, m_btnSelectOneInsCo);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnesthesiaInsCoSetupDlg, CNxDialog)
	//{{AFX_MSG_MAP(CAnesthesiaInsCoSetupDlg)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_ANESTH_INSCO, OnBtnSelectOneAnesthInsco)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL_ANESTH_INSCO, OnBtnSelectAllAnesthInsco)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_ANESTH_INSCO, OnBtnUnselectOneAnesthInsco)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_ANESTH_INSCO, OnBtnUnselectAllAnesthInsco)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnesthesiaInsCoSetupDlg message handlers

BOOL CAnesthesiaInsCoSetupDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	// (z.manning, 04/30/2008) - PLID 29850 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	m_UnselectedList = BindNxDataListCtrl(this,IDC_UNSELECTED_ANESTH_INSCO_LIST,GetRemoteData(),true);
	m_SelectedList = BindNxDataListCtrl(this,IDC_SELECTED_ANESTH_INSCO_LIST,GetRemoteData(),true);

	m_btnSelectOneInsCo.AutoSet(NXB_RIGHT);
	m_btnSelectAllInsCo.AutoSet(NXB_RRIGHT);
	m_btnUnselectOneInsCo.AutoSet(NXB_LEFT);
	m_btnUnselectAllInsCo.AutoSet(NXB_LLEFT);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAnesthesiaInsCoSetupDlg::OnBtnSelectOneAnesthInsco() 
{
	m_SelectedList->TakeCurrentRow(m_UnselectedList);	
}

void CAnesthesiaInsCoSetupDlg::OnBtnSelectAllAnesthInsco() 
{
	m_SelectedList->TakeAllRows(m_UnselectedList);
}

void CAnesthesiaInsCoSetupDlg::OnBtnUnselectOneAnesthInsco() 
{
	m_UnselectedList->TakeCurrentRow(m_SelectedList);
}

void CAnesthesiaInsCoSetupDlg::OnBtnUnselectAllAnesthInsco() 
{
	m_UnselectedList->TakeAllRows(m_SelectedList);	
}

BEGIN_EVENTSINK_MAP(CAnesthesiaInsCoSetupDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAnesthesiaInsCoSetupDlg)
	ON_EVENT(CAnesthesiaInsCoSetupDlg, IDC_UNSELECTED_ANESTH_INSCO_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedAnesthInscoList, VTS_I4 VTS_I2)
	ON_EVENT(CAnesthesiaInsCoSetupDlg, IDC_SELECTED_ANESTH_INSCO_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedAnesthInscoList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAnesthesiaInsCoSetupDlg::OnDblClickCellUnselectedAnesthInscoList(long nRowIndex, short nColIndex) 
{
	m_UnselectedList->CurSel = nRowIndex;

	if(nRowIndex != -1)
		OnBtnSelectOneAnesthInsco();
}

void CAnesthesiaInsCoSetupDlg::OnDblClickCellSelectedAnesthInscoList(long nRowIndex, short nColIndex) 
{
	m_SelectedList->CurSel = nRowIndex;

	if(nRowIndex != -1)
		OnBtnUnselectOneAnesthInsco();
}

void CAnesthesiaInsCoSetupDlg::OnOK() 
{
	try {

		ExecuteSql("UPDATE InsuranceCoT SET AnesthesiaSetting = 0");

		for(int i=0;i<m_SelectedList->GetRowCount();i++) {
			ExecuteSql("UPDATE InsuranceCoT SET AnesthesiaSetting = 1 WHERE PersonID = %li",VarLong(m_SelectedList->GetValue(i,0)));
		}
	
		CDialog::OnOK();

	}NxCatchAll("Error saving changes.");
}
