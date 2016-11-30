// ExportWizardTodoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "financialrc.h"
#include "ExportWizardTodoDlg.h"
#include "ExportWizardDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CExportWizardTodoDlg property page

IMPLEMENT_DYNCREATE(CExportWizardTodoDlg, CPropertyPage)

CExportWizardTodoDlg::CExportWizardTodoDlg() : CPropertyPage(CExportWizardTodoDlg::IDD)
{
	//{{AFX_DATA_INIT(CExportWizardTodoDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CExportWizardTodoDlg::~CExportWizardTodoDlg()
{
}

void CExportWizardTodoDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CExportWizardTodoDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_EXPORT_INTERVAL_NUMBER, m_nxeditExportIntervalNumber);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CExportWizardTodoDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CExportWizardTodoDlg)
	ON_BN_CLICKED(IDC_EXPORT_RUN_MANUAL, OnExportRunManual)
	ON_BN_CLICKED(IDC_EXPORT_USE_INTERVAL, OnExportUseInterval)
	ON_EN_CHANGE(IDC_EXPORT_INTERVAL_NUMBER, OnChangeExportIntervalNumber)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExportWizardTodoDlg message handlers

BOOL CExportWizardTodoDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_pDateUnits = BindNxDataListCtrl(this, IDC_EXPORT_INTERVAL_UNIT, NULL, false);
	IRowSettingsPtr pRow = m_pDateUnits->GetRow(-1);
	pRow->PutValue(0, (long)diDays);
	pRow->PutValue(1, _bstr_t("Days"));
	m_pDateUnits->AddRow(pRow);
	pRow = m_pDateUnits->GetRow(-1);
	pRow->PutValue(0, (long)diWeeks);
	pRow->PutValue(1, _bstr_t("Weeks"));
	m_pDateUnits->AddRow(pRow);
	pRow = m_pDateUnits->GetRow(-1);
	pRow->PutValue(0, (long)diMonths);
	pRow->PutValue(1, _bstr_t("Months"));
	m_pDateUnits->AddRow(pRow);
	m_pUserList = BindNxDataListCtrl(this, IDC_EXPORT_USER_LIST, GetRemoteData(), true);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CExportWizardTodoDlg, CPropertyPage)
    //{{AFX_EVENTSINK_MAP(CExportWizardTodoDlg)
	ON_EVENT(CExportWizardTodoDlg, IDC_EXPORT_USER_LIST, 16 /* SelChosen */, OnSelChosenExportUserList, VTS_I4)
	ON_EVENT(CExportWizardTodoDlg, IDC_EXPORT_INTERVAL_UNIT, 16 /* SelChosen */, OnSelChosenExportIntervalUnit, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CExportWizardTodoDlg::OnExportRunManual() 
{
	EnableFields();

	((CExportWizardDlg*)GetParent())->m_bCreateTodo = false;
}

void CExportWizardTodoDlg::OnExportUseInterval() 
{
	EnableFields();

	((CExportWizardDlg*)GetParent())->m_bCreateTodo = true;
	OnSelChosenExportUserList(m_pUserList->CurSel);
}

void CExportWizardTodoDlg::OnSelChosenExportUserList(long nRow) 
{
	if(nRow == -1) {
		m_pUserList->CurSel = 0;
		OnSelChosenExportUserList(0);
	}
	else {
		((CExportWizardDlg*)GetParent())->m_nTodoUser = VarLong(m_pUserList->GetValue(nRow,0));
	}
}

void CExportWizardTodoDlg::OnChangeExportIntervalNumber() 
{
	((CExportWizardDlg*)GetParent())->m_nTodoIntervalAmount = GetDlgItemInt(IDC_EXPORT_INTERVAL_NUMBER);	
}

void CExportWizardTodoDlg::OnSelChosenExportIntervalUnit(long nRow) 
{
	if(nRow == -1) {
		m_pDateUnits->CurSel = 0;
	}
	else {
		((CExportWizardDlg*)GetParent())->m_diTodoIntervalUnit = (DateInterval)VarLong(m_pDateUnits->GetValue(nRow,0));
	}
}

BOOL CExportWizardTodoDlg::OnSetActive()
{
	if(((CExportWizardDlg*)GetParent())->m_bCreateTodo) {
		CheckRadioButton(IDC_EXPORT_RUN_MANUAL, IDC_EXPORT_USE_INTERVAL, IDC_EXPORT_USE_INTERVAL);
		//TES 6/27/2007 - PLID 26435 - We were calling OnExportUseInterval here, but that did too much (including setting
		// the user), we just want to enable the appropriate fields.
		EnableFields();
	}
	else {
		CheckRadioButton(IDC_EXPORT_RUN_MANUAL, IDC_EXPORT_USE_INTERVAL, IDC_EXPORT_RUN_MANUAL);
		//TES 6/27/2007 - PLID 26435 - As in the above branch, just make sure the appropriate fields are enabled, don't
		// call the handler for this radio button.
		EnableFields();
	}

	SetDlgItemInt(IDC_EXPORT_INTERVAL_NUMBER, ((CExportWizardDlg*)GetParent())->m_nTodoIntervalAmount);
	if(-1 == m_pDateUnits->FindByColumn(0, (long)((CExportWizardDlg*)GetParent())->m_diTodoIntervalUnit, 0, VARIANT_TRUE)) {
		m_pDateUnits->CurSel = 0;
	}
	if(-1 == m_pUserList->FindByColumn(0, ((CExportWizardDlg*)GetParent())->m_nTodoUser, 0, VARIANT_TRUE)) {
		m_pUserList->CurSel = 0;
	}

	//They can always go next or back.
	((CExportWizardDlg*)GetParent())->SetWizardButtons(PSWIZB_BACK|PSWIZB_NEXT);

	((CExportWizardDlg*)GetParent())->SetTitle("How often should export \"" + ((CExportWizardDlg*)GetParent())->m_strName + "\" be run?");

	return CPropertyPage::OnSetActive();
}


void CExportWizardTodoDlg::EnableFields()
{
	//TES 6/27/2007 - PLID 26435 - Make sure that all fields are enabled appropriately, based on which buttons are checked.
	GetDlgItem(IDC_EXPORT_INTERVAL_NUMBER)->EnableWindow(IsDlgButtonChecked(IDC_EXPORT_USE_INTERVAL));
	GetDlgItem(IDC_EXPORT_INTERVAL_UNIT)->EnableWindow(IsDlgButtonChecked(IDC_EXPORT_USE_INTERVAL));
	GetDlgItem(IDC_EXPORT_USER_LIST)->EnableWindow(IsDlgButtonChecked(IDC_EXPORT_USE_INTERVAL));
}
