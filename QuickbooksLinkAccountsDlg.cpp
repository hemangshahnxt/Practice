// QuickbooksLinkAccountsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "QuickbooksLinkAccountsDlg.h"
#include "QuickbooksUtils.h"
#include "GlobalDrawingUtils.h"

// (a.walling 2012-08-03 14:31) - PLID 51956 - Compiler limits exceeded with imported NxAccessor typelib - get quickbooks out of stdafx
// (a.walling 2014-04-30 15:19) - PLID 61989 - import a typelibrary rather than a dll
#import "QBFC3.tlb" no_namespace

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CQuickbooksLinkAccountsDlg dialog


CQuickbooksLinkAccountsDlg::CQuickbooksLinkAccountsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CQuickbooksLinkAccountsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CQuickbooksLinkAccountsDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CQuickbooksLinkAccountsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CQuickbooksLinkAccountsDlg)
	DDX_Control(pDX, IDC_BTN_UNLINK_ALL_PROVS, m_btnUnlinkAllProvs);
	DDX_Control(pDX, IDC_BTN_UNLINK_ONE_PROV, m_btnUnlinkOneProv);
	DDX_Control(pDX, IDC_BTN_LINK_ONE_PROV, m_btnLinkOneProv);
	DDX_Control(pDX, IDC_BTN_LINK_ALL_PROVS, m_btnLinkAllProvs);
	DDX_Control(pDX, IDOK, m_btnClose);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CQuickbooksLinkAccountsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CQuickbooksLinkAccountsDlg)
	ON_BN_CLICKED(IDC_BTN_LINK_ALL_PROVS, OnBtnLinkAllProvs)
	ON_BN_CLICKED(IDC_BTN_LINK_ONE_PROV, OnBtnLinkOneProv)
	ON_BN_CLICKED(IDC_BTN_UNLINK_ONE_PROV, OnBtnUnlinkOneProv)
	ON_BN_CLICKED(IDC_BTN_UNLINK_ALL_PROVS, OnBtnUnlinkAllProvs)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CQuickbooksLinkAccountsDlg message handlers

BOOL CQuickbooksLinkAccountsDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();
	
	m_btnLinkAllProvs.AutoSet(NXB_RRIGHT);
	m_btnLinkOneProv.AutoSet(NXB_RIGHT);
	m_btnUnlinkOneProv.AutoSet(NXB_LEFT);
	m_btnUnlinkAllProvs.AutoSet(NXB_LLEFT);
	m_btnClose.AutoSet(NXB_CLOSE);

	m_brush.CreateSolidBrush(PaletteColor(0x009ED6BA));

	m_AccountCombo = BindNxDataListCtrl(this,IDC_DEPOSIT_ACCOUNTS_COMBO,GetRemoteData(),false);
	m_UnselectedProviderList = BindNxDataListCtrl(this,IDC_QB_PROVIDER_UNSELECTED_LIST,GetRemoteData(),true);
	m_SelectedProviderList = BindNxDataListCtrl(this,IDC_QB_PROVIDER_SELECTED_LIST,GetRemoteData(),false);

	IQBSessionManagerPtr qb = QB_OpenSession();

	if(qb != NULL) {

		// Create the request manager
		IMsgSetRequestPtr pReqSet = QB_CreateMsgSetRequest(qb);
		pReqSet->Attributes->OnError = roeContinue;

		// Create and prepare the request for the list all bank accounts
		IAccountQueryPtr pAcctsReq = pReqSet->AppendAccountQueryRq();
		IAccountListFilterPtr pFilter = pAcctsReq->ORAccountListQuery->AccountListFilter;
		pFilter->ActiveStatus->SetValue(asActiveOnly);
		pFilter->AccountTypeList->Add(atBank);
		pFilter->AccountTypeList->Add(atOtherCurrentAsset);
		
		// Do the request, and get the result
		IResponsePtr pResp = qb->DoRequests(pReqSet)->ResponseList->GetAt(0);
		if (pResp->StatusCode == 0) {
			// The result was success, so parse the return value to get the list of accounts
			IAccountRetListPtr pAcctsRetList = pResp->Detail;
			long nAcctCount = pAcctsRetList->GetCount();
			for (long i=0; i<nAcctCount; i++) {
				IAccountRetPtr pAcctRet = pAcctsRetList->GetAt(i);

				IRowSettingsPtr pRow = m_AccountCombo->GetRow(-1);
				pRow->PutValue(0,_variant_t((LPCTSTR)pAcctRet->ListID->GetValue()));
				pRow->PutValue(1,_variant_t((LPCTSTR)pAcctRet->Name->GetValue()));
				m_AccountCombo->AddRow(pRow);
			}
		}

		//if we did successfully connect to QuickBooks but there were no valid accounts found, warn them as such
		if(m_AccountCombo->GetRowCount() == 0) {
			AfxMessageBox("No valid deposit accounts were found in your QuickBooks database.\n"
				"You will not be able to link accounts, or send payments, until a deposit account exists.");
		}
		else {
			m_AccountCombo->PutCurSel(0);
			OnSelChosenDepositAccountsCombo(0);
		}

		//after loading the accounts (successfully), look for links we may have to accounts that no longer exist
		CString strWhere = "";
		for(int i=0;i<m_AccountCombo->GetRowCount();i++) {
			CString str;
			str.Format(" QBooksAcctID <> '%s' AND",_Q(CString(m_AccountCombo->GetValue(i,0).bstrVal)));
			strWhere += str;
		}
		if(strWhere.GetLength() > 0) {
			strWhere.TrimRight("AND");
			_RecordsetPtr rs = CreateRecordset("SELECT QBooksAcctID, QBooksAcctName FROM QBooksAcctsToProvidersT WHERE %s GROUP BY QBooksAcctID, QBooksAcctName",strWhere);
			while(!rs->eof) {
				CString strAcctID = AdoFldString(rs, "QBooksAcctID","");
				CString strAcctName = AdoFldString(rs, "QBooksAcctName","");
				CString str;
				str.Format("There are providers linked to account '%s' which appears to no longer exist in QuickBooks.\n"
					"Do you wish to remove this link now?\n\n"
					"If you do not remove this link, the providers cannot be re-linked to other accounts,\n"
					"and will always prompt for a deposit account.",strAcctName);
				if(IDYES == MessageBox(str,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
					ExecuteSql("DELETE FROM QBooksAcctsToProvidersT WHERE QBooksAcctID = '%s'",_Q(strAcctID));
					m_UnselectedProviderList->Requery();
				}
				rs->MoveNext();				
			}
			rs->Close();
		}

		qb->EndSession();
	}
	else {
		AfxMessageBox("The account linking can not be configured without a valid connection to QuickBooks.\n"
			"Please set up Quickbooks properly before using this feature.");
	}	
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CQuickbooksLinkAccountsDlg::OnBtnLinkAllProvs() 
{
	try{
		if(m_AccountCombo->CurSel==-1)
			return;

		long i = 0;
		long p = m_UnselectedProviderList->GetFirstRowEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{	i++;
			m_UnselectedProviderList->GetNextRowEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			ExecuteSql("INSERT INTO QBooksAcctsToProvidersT (ProviderID, QBooksAcctID, QBooksAcctName) VALUES (%li,'%s','%s')",
				pRow->GetValue(0).lVal,CString(m_AccountCombo->GetValue(m_AccountCombo->CurSel,0).bstrVal),CString(m_AccountCombo->GetValue(m_AccountCombo->CurSel,1).bstrVal));

			pDisp->Release();
		}

		m_SelectedProviderList->TakeAllRows(m_UnselectedProviderList);

	}NxCatchAll("Error in OnAddAll()");
}

void CQuickbooksLinkAccountsDlg::OnBtnLinkOneProv() 
{
	CWaitCursor pWait;

	try{
		if(m_AccountCombo->CurSel==-1 || m_UnselectedProviderList->CurSel == -1)
			return;
		
		long i = 0;
		long p = m_UnselectedProviderList->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{	i++;
			m_UnselectedProviderList->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			ExecuteSql("INSERT INTO QBooksAcctsToProvidersT (ProviderID, QBooksAcctID, QBooksAcctName) VALUES (%li,'%s','%s')",
				pRow->GetValue(0).lVal,CString(m_AccountCombo->GetValue(m_AccountCombo->CurSel,0).bstrVal),CString(m_AccountCombo->GetValue(m_AccountCombo->CurSel,1).bstrVal));

			pDisp->Release();
		}

		m_SelectedProviderList->TakeCurrentRow(m_UnselectedProviderList);

	}NxCatchAll("Error in OnAddCompany()");
}

void CQuickbooksLinkAccountsDlg::OnBtnUnlinkOneProv() 
{
	CWaitCursor pWait;

	try{
		if(m_AccountCombo->CurSel==-1 || m_SelectedProviderList->CurSel == -1)
			return;

		long i = 0;
		long p = m_SelectedProviderList->GetFirstSelEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{	i++;
			m_SelectedProviderList->GetNextSelEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			ExecuteSql("DELETE FROM QBooksAcctsToProvidersT WHERE ProviderID = %li;", pRow->GetValue(0).lVal);

			pDisp->Release();
		}

		m_UnselectedProviderList->TakeCurrentRow(m_SelectedProviderList);

	}NxCatchAll("Error in OnRemoveCompany");
}

void CQuickbooksLinkAccountsDlg::OnBtnUnlinkAllProvs() 
{
	CWaitCursor pWait;

	try{
		if(m_AccountCombo->CurSel==-1)
			return;

		long i = 0;
		long p = m_SelectedProviderList->GetFirstRowEnum();

		LPDISPATCH pDisp = NULL;

		while (p)
		{	i++;
			m_SelectedProviderList->GetNextRowEnum(&p, &pDisp);

			IRowSettingsPtr pRow(pDisp);

			ExecuteSql("DELETE FROM QBooksAcctsToProvidersT WHERE ProviderID = %li;", pRow->GetValue(0).lVal);

			pDisp->Release();
		}

		m_UnselectedProviderList->TakeAllRows(m_SelectedProviderList);

	}NxCatchAll("Error in OnRemoveAllCompanies()");
}

BEGIN_EVENTSINK_MAP(CQuickbooksLinkAccountsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CQuickbooksLinkAccountsDlg)
	ON_EVENT(CQuickbooksLinkAccountsDlg, IDC_DEPOSIT_ACCOUNTS_COMBO, 16 /* SelChosen */, OnSelChosenDepositAccountsCombo, VTS_I4)
	ON_EVENT(CQuickbooksLinkAccountsDlg, IDC_QB_PROVIDER_UNSELECTED_LIST, 3 /* DblClickCell */, OnDblClickCellQbProviderUnselectedList, VTS_I4 VTS_I2)
	ON_EVENT(CQuickbooksLinkAccountsDlg, IDC_QB_PROVIDER_SELECTED_LIST, 3 /* DblClickCell */, OnDblClickCellQbProviderSelectedList, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CQuickbooksLinkAccountsDlg::OnSelChosenDepositAccountsCombo(long nRow) 
{
	try {

		if(nRow == -1) {
			m_SelectedProviderList->Clear();
			return;
		}

		CString str;
		str.Format("ID IN (SELECT ProviderID FROM QBooksAcctsToProvidersT WHERE QBooksAcctID = '%s') AND PersonT.Archived = 0",_Q(CString(m_AccountCombo->GetValue(m_AccountCombo->CurSel,0).bstrVal)));
		m_SelectedProviderList->PutWhereClause(_bstr_t(str));
		m_SelectedProviderList->Requery();			
		
	}NxCatchAll("Error switching to Quickbooks account.");
}

void CQuickbooksLinkAccountsDlg::OnDblClickCellQbProviderUnselectedList(long nRowIndex, short nColIndex) 
{
	OnBtnLinkOneProv();
}

void CQuickbooksLinkAccountsDlg::OnDblClickCellQbProviderSelectedList(long nRowIndex, short nColIndex) 
{
	OnBtnUnlinkOneProv();
}
