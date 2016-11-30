// CredentialsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CredentialsDlg.h"
#include "GlobalDrawingUtils.h"
#include "ConfigureLicensesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


const int BACKGROUND_COLOR  = GetNxColor(GNC_ADMIN, 0);
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CCredentialsDlg dialog


CCredentialsDlg::CCredentialsDlg(CWnd* pParent)
	: CNxDialog(CCredentialsDlg::IDD, pParent),
	m_providerChecker(NetUtils::Providers),
	m_cptChecker(NetUtils::CPTCodeT)
{
	//{{AFX_DATA_INIT(CCredentialsDlg)
	//}}AFX_DATA_INIT

	//(j.anspach 06-09-2005 10:26 PLID 16662) - Updating the help files to incorporate the new help .chm
	//PLID 21512: per Don, if we don't have anything to put here, default to the earliest thing we can which is new patient
	//m_strManualLocation = "NexTech_Practice_Manual.chm";
	//m_strManualBookmark = "Surgery_Center/Credentials.htm";
}


void CCredentialsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCredentialsDlg)
	DDX_Control(pDX, IDC_BTN_CONFIGURE_LICENSES, m_btnConfigureLicenses);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_CPT, m_btnUnselectAllCPT);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_CPT, m_btnUnselectOneCPT);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL_CPT, m_btnSelectAllCPT);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_CPT, m_btnSelectOneCPT);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ALL_PROCEDURES, m_btnUnselectAllProcedures);
	DDX_Control(pDX, IDC_BTN_UNSELECT_ONE_PROCEDURE, m_btnUnselectOneProcedure);
	DDX_Control(pDX, IDC_BTN_SELECT_ALL_PROCEDURES, m_btnSelectAllProcedures);
	DDX_Control(pDX, IDC_BTN_SELECT_ONE_PROCEDURE, m_btnSelectOneProcedure);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCredentialsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CCredentialsDlg)
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_CPT, OnBtnSelectOneCpt)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL_CPT, OnBtnSelectAllCpt)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_CPT, OnBtnUnselectOneCpt)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_CPT, OnBtnUnselectAllCpt)
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(IDC_BTN_SELECT_ONE_PROCEDURE, OnBtnSelectOneProcedure)
	ON_BN_CLICKED(IDC_BTN_SELECT_ALL_PROCEDURES, OnBtnSelectAllProcedures)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ONE_PROCEDURE, OnBtnUnselectOneProcedure)
	ON_BN_CLICKED(IDC_BTN_UNSELECT_ALL_PROCEDURES, OnBtnUnselectAllProcedures)
	ON_BN_CLICKED(IDC_BTN_CONFIGURE_LICENSES, OnBtnConfigureLicenses)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCredentialsDlg message handlers

BOOL CCredentialsDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));
		
		m_ProviderCombo = BindNxDataListCtrl(IDC_PROVIDER_CREDENTIAL_COMBO);
		m_UnselectedCPTList = BindNxDataListCtrl(IDC_UNSELECTED_CPT_LIST, false);
		m_SelectedCPTList = BindNxDataListCtrl(IDC_SELECTED_CPT_LIST, false);
		m_UnselectedProcedureList = BindNxDataListCtrl(IDC_UNSELECTED_PROCEDURE_LIST, false);
		m_SelectedProcedureList = BindNxDataListCtrl(IDC_SELECTED_PROCEDURE_LIST, false);

		m_btnSelectOneCPT.AutoSet(NXB_RIGHT);
		m_btnUnselectOneCPT.AutoSet(NXB_LEFT);
		m_btnSelectAllCPT.AutoSet(NXB_RRIGHT);
		m_btnUnselectAllCPT.AutoSet(NXB_LLEFT);

		m_btnSelectOneProcedure.AutoSet(NXB_RIGHT);
		m_btnUnselectOneProcedure.AutoSet(NXB_LEFT);
		m_btnSelectAllProcedures.AutoSet(NXB_RRIGHT);
		m_btnUnselectAllProcedures.AutoSet(NXB_LLEFT);

		m_btnConfigureLicenses.AutoSet(NXB_MODIFY); // (z.manning, 05/13/2008) - PLID 29852

		m_ProviderCombo->PutCurSel(0);

		EnableControls();

	}NxCatchAll("Error initializing tab.");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CCredentialsDlg::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{
	if (m_providerChecker.Changed())
	{
		m_ProviderCombo->Requery();
		m_ProviderCombo->CurSel = 0;
		//don't bother, it's going to refresh anyways in Load()
		//if(m_ProviderCombo->CurSel!=-1)
		//	OnSelChosenProviderCredentialCombo(0);
	}
	if (m_cptChecker.Changed())
	{
		//don't bother, it's going to refresh anyways in Load()
		//if(m_ProviderCombo->CurSel != -1)
		//	OnSelChosenProviderCredentialCombo(m_ProviderCombo);
	}

	Load();
}

void CCredentialsDlg::Load()
{
	try {

		CWaitCursor pWait;

		if(m_ProviderCombo->CurSel==-1)
			return;

		EnableControls();
		
		long ProvID = m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0).lVal;

		CString strWhere;
		strWhere.Format("Active = 1 AND CPTCodeT.ID NOT IN (SELECT CPTCodeID FROM CredentialedCPTCodesT WHERE ProviderID = %li)", ProvID);
		m_UnselectedCPTList->WhereClause = (LPCTSTR)strWhere;
		m_UnselectedCPTList->Requery();

		strWhere.Format("Active = 1 AND CPTCodeT.ID IN (SELECT CPTCodeID FROM CredentialedCPTCodesT WHERE ProviderID = %li)", ProvID);
		m_SelectedCPTList->WhereClause = (LPCTSTR)strWhere;
		m_SelectedCPTList->Requery();

		// (c.haag 2008-12-18 17:23) - PLID 32518 - Hide inactive procedures
		strWhere.Format("Inactive = 0 AND ID NOT IN (SELECT ProcedureID FROM CredentialedProceduresT WHERE ProviderID = %li)", ProvID);
		m_UnselectedProcedureList->WhereClause = (LPCTSTR)strWhere;
		m_UnselectedProcedureList->Requery();

		strWhere.Format("ID IN (SELECT ProcedureID FROM CredentialedProceduresT WHERE ProviderID = %li)", ProvID);
		m_SelectedProcedureList->WhereClause = (LPCTSTR)strWhere;
		m_SelectedProcedureList->Requery();

	}NxCatchAll("Error loading credentials.");
}

void CCredentialsDlg::OnBtnSelectOneCpt() 
{
	try {

		if(!CheckCurrentUserPermissions(bioCredentialsTab,sptWrite)) {
			return;
		}

		CWaitCursor pWait;

		if(m_ProviderCombo->CurSel==-1 || m_UnselectedCPTList->CurSel == -1)
			return;
		
		long i = 0;
		long p = m_UnselectedCPTList->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{	i++;
			m_UnselectedCPTList->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			ExecuteSql("INSERT INTO CredentialedCPTCodesT (ProviderID, CPTCodeID) VALUES (%li,%li)", m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0).lVal, pRow->GetValue(0).lVal);

			pDisp->Release();
		}

		m_SelectedCPTList->TakeCurrentRow(m_UnselectedCPTList);

	}NxCatchAll("Error selecting service codes.");
}

void CCredentialsDlg::OnBtnSelectAllCpt() 
{
	try {
		if (m_ProviderCombo->CurSel==-1)
			return;

		if(!CheckCurrentUserPermissions(bioCredentialsTab,sptWrite)) {
			return;
		}

		CWaitCursor pWait;

		ExecuteSql("INSERT INTO CredentialedCPTCodesT (ProviderID, CPTCodeID) SELECT %li, ID FROM CPTCodeT WHERE ID NOT IN (SELECT CPTCodeID FROM CredentialedCPTCodesT WHERE ProviderID = %li)", m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0).lVal, m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0).lVal);
		m_SelectedCPTList->TakeAllRows(m_UnselectedCPTList);

	}NxCatchAll("Error selecting all service codes.");
}

void CCredentialsDlg::OnBtnUnselectOneCpt() 
{
	try {

		if(!CheckCurrentUserPermissions(bioCredentialsTab,sptWrite)) {
			return;
		}

		CWaitCursor pWait;

		if(m_ProviderCombo->CurSel==-1 || m_SelectedCPTList->CurSel == -1)
			return;
		
		long i = 0;
		long p = m_SelectedCPTList->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{	i++;
			m_SelectedCPTList->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			ExecuteSql("DELETE FROM CredentialedCPTCodesT WHERE ProviderID = %li AND CPTCodeID = %li", m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0).lVal, pRow->GetValue(0).lVal);

			pDisp->Release();
		}

		m_UnselectedCPTList->TakeCurrentRow(m_SelectedCPTList);

	}NxCatchAll("Error unselecting service codes.");
}

void CCredentialsDlg::OnBtnUnselectAllCpt() 
{
	try {
		if (m_ProviderCombo->CurSel==-1)
			return;

		if(!CheckCurrentUserPermissions(bioCredentialsTab,sptWrite)) {
			return;
		}

		CWaitCursor pWait;

		ExecuteSql("DELETE FROM CredentialedCPTCodesT WHERE ProviderID = %li", m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0).lVal);
		m_UnselectedCPTList->TakeAllRows(m_SelectedCPTList);

	}NxCatchAll("Error unselecting all service codes.");
}

BEGIN_EVENTSINK_MAP(CCredentialsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CCredentialsDlg)
	ON_EVENT(CCredentialsDlg, IDC_PROVIDER_CREDENTIAL_COMBO, 16 /* SelChosen */, OnSelChosenProviderCredentialCombo, VTS_I4)
	ON_EVENT(CCredentialsDlg, IDC_UNSELECTED_CPT_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedCptList, VTS_I4 VTS_I2)
	ON_EVENT(CCredentialsDlg, IDC_SELECTED_CPT_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedCptList, VTS_I4 VTS_I2)
	ON_EVENT(CCredentialsDlg, IDC_UNSELECTED_PROCEDURE_LIST, 3 /* DblClickCell */, OnDblClickCellUnselectedProcedureList, VTS_I4 VTS_I2)
	ON_EVENT(CCredentialsDlg, IDC_SELECTED_PROCEDURE_LIST, 3 /* DblClickCell */, OnDblClickCellSelectedProcedureList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CCredentialsDlg::OnSelChosenProviderCredentialCombo(long nRow) 
{
	// (a.walling 2006-10-05 15:20) - PLID 22848 - Fix errors when no provider selected.
	if (nRow == sriNoRow) {
		m_ProviderCombo->PutCurSel(0);
		// same as InitDialog
	}
	Load();	
}

void CCredentialsDlg::OnDblClickCellUnselectedCptList(long nRowIndex, short nColIndex) 
{
	try {

		m_UnselectedCPTList->CurSel = nRowIndex;

		OnBtnSelectOneCpt();

	}NxCatchAll("Error selecting service code.");
}

void CCredentialsDlg::OnDblClickCellSelectedCptList(long nRowIndex, short nColIndex) 
{
	try {

		m_SelectedCPTList->CurSel = nRowIndex;

		OnBtnUnselectOneCpt();

	}NxCatchAll("Error unselecting service code.");
}

HBRUSH CCredentialsDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	/*
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);

	if (nCtlColor == CTLCOLOR_STATIC)
	{
		extern CPracticeApp theApp;
		pDC->SelectPalette(&theApp.m_palette, FALSE);
		pDC->RealizePalette();
		pDC->SetBkColor(PaletteColor(0x008080FF));
		return m_brush;
	}
	return hbr;
	*/

	// (a.walling 2008-04-01 16:47) - PLID 29497 - Deprecated; use parent class' implementation
	return CNxDialog::OnCtlColor(pDC, pWnd, nCtlColor);
}

void CCredentialsDlg::OnDblClickCellUnselectedProcedureList(long nRowIndex, short nColIndex) 
{
	try {

		m_UnselectedProcedureList->CurSel = nRowIndex;

		OnBtnSelectOneProcedure();

	}NxCatchAll("Error selecting procedure.");
}

void CCredentialsDlg::OnDblClickCellSelectedProcedureList(long nRowIndex, short nColIndex) 
{
	try {

		m_SelectedProcedureList->CurSel = nRowIndex;

		OnBtnUnselectOneProcedure();

	}NxCatchAll("Error unselecting procedure.");
}

void CCredentialsDlg::OnBtnSelectOneProcedure() 
{
	try {

		if(!CheckCurrentUserPermissions(bioCredentialsTab,sptWrite)) {
			return;
		}

		CWaitCursor pWait;

		if(m_ProviderCombo->CurSel==-1 || m_UnselectedProcedureList->CurSel == -1)
			return;
		
		long i = 0;
		long p = m_UnselectedProcedureList->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{	i++;
			m_UnselectedProcedureList->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			ExecuteSql("INSERT INTO CredentialedProceduresT (ProviderID, ProcedureID) VALUES (%li,%li)", m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0).lVal, pRow->GetValue(0).lVal);

			pDisp->Release();
		}

		m_SelectedProcedureList->TakeCurrentRow(m_UnselectedProcedureList);

	}NxCatchAll("Error selecting procedures.");
}

void CCredentialsDlg::OnBtnSelectAllProcedures() 
{
	try {
		if (m_ProviderCombo->CurSel==-1)
			return;

		if(!CheckCurrentUserPermissions(bioCredentialsTab,sptWrite)) {
			return;
		}

		CWaitCursor pWait;
		// (c.haag 2008-12-18 17:22) - PLID 32518 - Don't add inactive procedures
		ExecuteSql("INSERT INTO CredentialedProceduresT (ProviderID, ProcedureID) SELECT %li, ID FROM ProcedureT WHERE Inactive = 0 AND ID NOT IN (SELECT ProcedureID FROM CredentialedProceduresT WHERE ProviderID = %li)", m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0).lVal, m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0).lVal);
		m_SelectedProcedureList->TakeAllRows(m_UnselectedProcedureList);

	}NxCatchAll("Error selecting all procedures.");
}

void CCredentialsDlg::OnBtnUnselectOneProcedure() 
{
	try {

		if(!CheckCurrentUserPermissions(bioCredentialsTab,sptWrite)) {
			return;
		}

		CWaitCursor pWait;

		if(m_ProviderCombo->CurSel==-1 || m_SelectedProcedureList->CurSel == -1)
			return;
		
		long i = 0;
		long p = m_SelectedProcedureList->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{	i++;
			m_SelectedProcedureList->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			ExecuteSql("DELETE FROM CredentialedProceduresT WHERE ProviderID = %li AND ProcedureID = %li", m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0).lVal, pRow->GetValue(0).lVal);

			pDisp->Release();
		}

		m_UnselectedProcedureList->TakeCurrentRow(m_SelectedProcedureList);

	}NxCatchAll("Error unselecting procedures.");
}

void CCredentialsDlg::OnBtnUnselectAllProcedures() 
{
	try {
		if (m_ProviderCombo->CurSel==-1)
			return;

		if(!CheckCurrentUserPermissions(bioCredentialsTab,sptWrite)) {
			return;
		}

		CWaitCursor pWait;

		ExecuteSql("DELETE FROM CredentialedProceduresT WHERE ProviderID = %li", m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0).lVal);
		m_UnselectedProcedureList->TakeAllRows(m_SelectedProcedureList);

	}NxCatchAll("Error unselecting all procedures.");
}

void CCredentialsDlg::OnBtnConfigureLicenses() 
{
	if(!CheckCurrentUserPermissions(bioASCLicensing,sptRead)) {
		return;
	}

	CConfigureLicensesDlg dlg(this);

	if(m_ProviderCombo->CurSel != -1) {		
		dlg.m_nPersonID = VarLong(m_ProviderCombo->GetValue(m_ProviderCombo->CurSel,0),-1);
	}

	dlg.DoModal();
}

void CCredentialsDlg::EnableControls()
{
	BOOL bEnable = (GetCurrentUserPermissions(bioCredentialsTab) & (SPT___W________ANDPASS));

	GetDlgItem(IDC_BTN_UNSELECT_ALL_CPT)->EnableWindow(bEnable);
	GetDlgItem(IDC_BTN_UNSELECT_ONE_CPT)->EnableWindow(bEnable);
	GetDlgItem(IDC_BTN_SELECT_ALL_CPT)->EnableWindow(bEnable);
	GetDlgItem(IDC_BTN_SELECT_ONE_CPT)->EnableWindow(bEnable);
	GetDlgItem(IDC_BTN_UNSELECT_ALL_PROCEDURES)->EnableWindow(bEnable);
	GetDlgItem(IDC_BTN_UNSELECT_ONE_PROCEDURE)->EnableWindow(bEnable);
	GetDlgItem(IDC_BTN_SELECT_ALL_PROCEDURES)->EnableWindow(bEnable);
	GetDlgItem(IDC_BTN_SELECT_ONE_PROCEDURE)->EnableWindow(bEnable);
	m_UnselectedCPTList->PutReadOnly(!bEnable);
	m_SelectedCPTList->PutReadOnly(!bEnable);
	m_UnselectedProcedureList->PutReadOnly(!bEnable);
	m_SelectedProcedureList->PutReadOnly(!bEnable);


	//enable/disable the licenses button
	GetDlgItem(IDC_BTN_CONFIGURE_LICENSES)->EnableWindow(GetCurrentUserPermissions(bioASCLicensing) & (SPT__R_________ANDPASS));
}