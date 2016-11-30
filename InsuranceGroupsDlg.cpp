// InsuranceGroupsDlg1.cpp : implementation file
//

#include "stdafx.h"
#include "InsuranceGroupsDlg.h"
#include "GlobalDataUtils.h"
#include "AdvInsuranceGRPEdit.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37023 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CInsuranceGroupsDlg dialog


CInsuranceGroupsDlg::CInsuranceGroupsDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CInsuranceGroupsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CInsuranceGroupsDlg)
	m_iInsuranceCoID = -1;
	m_iProviderID = -1;
	m_GRP = _T("");
	//}}AFX_DATA_INIT
}


void CInsuranceGroupsDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInsuranceGroupsDlg)
	DDX_Text(pDX, IDC_EDIT_GRP, m_GRP);
	DDX_Control(pDX, IDC_EDIT_GRP, m_nxeditEditGrp);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ADV_GRP_EDIT, m_btnAdvGrpEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInsuranceGroupsDlg, CNxDialog)
	//{{AFX_MSG_MAP(CInsuranceGroupsDlg)
	ON_EN_KILLFOCUS(IDC_EDIT_GRP, OnKillfocusEditGrp)
	ON_BN_CLICKED(IDC_ADV_GRP_EDIT, OnAdvGrpEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInsuranceGroupsDlg message handlers

BOOL CInsuranceGroupsDlg::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-07 11:23) - PLID 29847 - NxIconified buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnAdvGrpEdit.AutoSet(NXB_MODIFY);

		// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - No g_ptrRemoteData
		m_pDocList = BindNxDataListCtrl(IDC_INS_DOCS, true);
		
		m_pDocList->CurSel = 0;
		m_iProviderID = VarLong(m_pDocList->GetValue(0, 0), -1);

		LoadGRPNumber();
	}
	NxCatchAll("Error in CInsuranceGroupsDlg::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CInsuranceGroupsDlg::LoadGRPNumber()
{
	_RecordsetPtr rs;
	_variant_t var;
	CString str;

	if (m_pDocList->GetRowCount() == 0) {
		m_GRP.Empty();
	}
	else try {
		str.Format("SELECT GRP FROM InsuranceGroups WHERE InsCoID = %d AND ProviderID = %d",
			m_iInsuranceCoID, m_iProviderID);
		rs = CreateRecordset("SELECT GRP FROM InsuranceGroups WHERE InsCoID = %li AND ProviderID = %li", m_iInsuranceCoID, m_iProviderID);

		if (!rs->eof) {
			var = rs->Fields->GetItem("GRP")->Value;
			if (var.vt == VT_NULL)
				m_GRP.Empty();
			else
				m_GRP = VarString(var, "");

		}
		else
			m_GRP.Empty();
		rs->Close();
	}NxCatchAll("Error in LoadGRPNumber()");

	UpdateData(FALSE);
}

void CInsuranceGroupsDlg::SaveGRPNumber()
{
	_RecordsetPtr rs;
	
	if (m_pDocList->GetRowCount() == 0)
		return;

	if(m_pDocList->CurSel==-1)
		return;

	try {
		//for auditing
		CString strOld;
		_RecordsetPtr rs = CreateRecordset("SELECT GRP FROM InsuranceGroups WHERE InsCoID = %d AND ProviderID = %d", m_iInsuranceCoID, m_iProviderID);
		if(!rs->eof && rs->Fields->Item["GRP"]->Value.vt == VT_BSTR) {
			strOld = CString(rs->Fields->Item["GRP"]->Value.bstrVal);
		}

		UpdateData(TRUE);

		rs = CreateRecordset("SELECT Count(InsCoID) AS GRPCount FROM InsuranceGroups WHERE InsCoID = %li AND ProviderID = %li", m_iInsuranceCoID, m_iProviderID);

		if (rs->Fields->GetItem("GRPCount")->Value.lVal == 0) {
			ExecuteSql("INSERT INTO InsuranceGroups (InsCoID, ProviderID, GRP) VALUES (%li, %li, '%s')", 
				m_iInsuranceCoID, m_iProviderID, _Q(m_GRP));
		}
		else{
			ExecuteSql("UPDATE InsuranceGroups SET GRP = '%s' WHERE InsCoID = %li AND ProviderID = %li",
				_Q(m_GRP), m_iInsuranceCoID, m_iProviderID);
		}
		//auditing
		CString strPerson;
		strPerson = CString(m_pDocList->GetValue(m_pDocList->CurSel, 1).bstrVal) + " / " + m_strInsCo;
		if(strOld != m_GRP) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, strPerson, nAuditID, aeiInsCoGroupNumber, -1, strOld, m_GRP, aepMedium, aetChanged);
		}
		rs->Close();
	}NxCatchAll("Error in SaveGRPNumber()");
}

BEGIN_EVENTSINK_MAP(CInsuranceGroupsDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CInsuranceGroupsDlg)
	ON_EVENT(CInsuranceGroupsDlg, IDC_INS_DOCS, 2 /* SelChanged */, OnSelChangedInsDocs, VTS_I4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()


void CInsuranceGroupsDlg::OnOK() 
{
	SaveGRPNumber();
	
	CNxDialog::OnOK();
}

void CInsuranceGroupsDlg::OnKillfocusEditGrp() 
{
	SaveGRPNumber();
}

void CInsuranceGroupsDlg::OnSelChangedInsDocs(long nNewSel) 
{
	m_iProviderID = VarLong(m_pDocList->GetValue(m_pDocList->CurSel, 0), -1);
	LoadGRPNumber();
}

void CInsuranceGroupsDlg::OnAdvGrpEdit() 
{
	CAdvInsuranceGRPEdit dlg(this);
	dlg.m_IDType = 1;
	dlg.DoModal();

	LoadGRPNumber();	
}
