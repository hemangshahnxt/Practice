// SurgeryProviderLinkDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SurgeryProviderLinkDlg.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;
using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CSurgeryProviderLinkDlg dialog


CSurgeryProviderLinkDlg::CSurgeryProviderLinkDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CSurgeryProviderLinkDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSurgeryProviderLinkDlg)
		m_nPreferenceCardID = -1;
	//}}AFX_DATA_INIT
}


void CSurgeryProviderLinkDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSurgeryProviderLinkDlg)
	DDX_Control(pDX, IDC_RADIO_SELECTED_SRGY_PROVIDERS, m_radioSelectedProviders);
	DDX_Control(pDX, IDC_RADIO_ALL_SRGY_PROVIDERS, m_radioAllProviders);
	DDX_Control(pDX, IDC_PREFERENCE_CARD_NAME_LABEL, m_nxstaticPreferenceCardNameLabel);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSurgeryProviderLinkDlg, CNxDialog)
	//{{AFX_MSG_MAP(CSurgeryProviderLinkDlg)
	ON_BN_CLICKED(IDC_RADIO_ALL_SRGY_PROVIDERS, OnRadioAllSrgyProviders)
	ON_BN_CLICKED(IDC_RADIO_SELECTED_SRGY_PROVIDERS, OnRadioSelectedSrgyProviders)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSurgeryProviderLinkDlg message handlers

BOOL CSurgeryProviderLinkDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	try {

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		// (j.jones 2009-08-24 11:41) - PLID 35124 - changed to preference cards
		_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM PreferenceCardsT WHERE ID = {INT}", m_nPreferenceCardID);
		if(!rs->eof)
			SetDlgItemText(IDC_PREFERENCE_CARD_NAME_LABEL,AdoFldString(rs, "Name",""));
		else
			SetDlgItemText(IDC_PREFERENCE_CARD_NAME_LABEL,"");
		rs->Close();
	
		m_ProviderList = BindNxDataListCtrl(this,IDC_SRGY_PROVIDER_LIST,GetRemoteData(),false);
		m_ProviderList->Requery();
		m_ProviderList->WaitForRequery(dlPatienceLevelWaitIndefinitely);

		rs = CreateParamRecordset("SELECT ProviderID FROM PreferenceCardProvidersT WHERE PreferenceCardID = {INT}", m_nPreferenceCardID);

		if(rs->eof) {
			//all providers
			m_radioAllProviders.SetCheck(TRUE);
			OnRadioAllSrgyProviders();
		}

		while(!rs->eof) {
			long ProviderID = AdoFldLong(rs, "ProviderID",-1);
			if(ProviderID == -1) {
				//all providers
				m_radioAllProviders.SetCheck(TRUE);
				OnRadioAllSrgyProviders();
			}
			else {
				//just selected providers
				m_radioSelectedProviders.SetCheck(TRUE);
				OnRadioSelectedSrgyProviders();
				m_ProviderList->PutValue(m_ProviderList->FindByColumn(0,ProviderID,0,FALSE),1,_variant_t(VARIANT_TRUE,VT_BOOL));
			}

			rs->MoveNext();
		}		

	}NxCatchAll("Error initializing CSurgeryProviderLinkDlg");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSurgeryProviderLinkDlg::OnOK() 
{
	// (j.jones 2009-08-24 11:41) - PLID 35124 - changed to preference cards
	BEGIN_TRANS("SavePreferenceCardProviderLink") {

		ExecuteParamSql("DELETE FROM PreferenceCardProvidersT WHERE PreferenceCardID = {INT}",m_nPreferenceCardID);
		if(m_radioSelectedProviders.GetCheck()) {
			//insert selected providers
			for(int i=0;i<m_ProviderList->GetRowCount();i++) {
				if(VarBool(m_ProviderList->GetValue(i,1),FALSE)) {
					ExecuteParamSql("INSERT INTO PreferenceCardProvidersT (PreferenceCardID, ProviderID) VALUES ({INT}, {INT})",m_nPreferenceCardID,VarLong(m_ProviderList->GetValue(i,0)));
				}
			}
		}


	} END_TRANS_CATCH_ALL("SavePreferenceCardProviderLink");
	
	CDialog::OnOK();
}

void CSurgeryProviderLinkDlg::OnRadioAllSrgyProviders() 
{
	if(m_radioAllProviders.GetCheck()) {
		m_ProviderList->CurSel = -1;
		m_ProviderList->Enabled = FALSE;
		int nSelCount = m_ProviderList->GetRowCount();
		for (long i=0; i<nSelCount; i++) {
			(IRowSettingsPtr(m_ProviderList->GetRow(i)))->PutForeColor(RGB(163, 163, 163));
		}
	}
}

void CSurgeryProviderLinkDlg::OnRadioSelectedSrgyProviders() 
{
	if(m_radioSelectedProviders.GetCheck()) {
		m_ProviderList->Enabled = TRUE;
		int nSelCount = m_ProviderList->GetRowCount();
		for (long i=0; i<nSelCount; i++) {
			(IRowSettingsPtr(m_ProviderList->GetRow(i)))->PutForeColor(RGB(0,0,0));
		}
	}		
}
