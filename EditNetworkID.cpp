// EditNetworkID.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "pracprops.h"
#include "AdvInsuranceGRPEdit.h"
#include "EditNetworkID.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CEditNetworkID dialog


CEditNetworkID::CEditNetworkID(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditNetworkID::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditNetworkID)
	m_NetworkID = _T("");
	//}}AFX_DATA_INIT
}


void CEditNetworkID::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditNetworkID)
	DDX_Text(pDX, IDC_EDIT_NETWORKID, m_NetworkID);
	DDV_MaxChars(pDX, m_NetworkID, 15);
	DDX_Control(pDX, IDC_EDIT_NETWORKID, m_nxeditEditNetworkid);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ADV_NETWORKID_EDIT, m_btnAdvNetworkIDEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditNetworkID, CNxDialog)
	//{{AFX_MSG_MAP(CEditNetworkID)
	ON_EN_KILLFOCUS(IDC_EDIT_NETWORKID, OnKillfocusEditNetworkid)
	ON_BN_CLICKED(IDC_ADV_NETWORKID_EDIT, OnAdvNetworkidEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditNetworkID message handlers

BOOL CEditNetworkID::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-07 11:09) - PLID 29817 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnAdvNetworkIDEdit.AutoSet(NXB_MODIFY);

		m_ComboProviders = BindNxDataListCtrl(this,IDC_COMBO_PROVIDERS,GetRemoteData(),true);
	}
	NxCatchAll("Error in CEditNetworkID::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEditNetworkID, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditNetworkID)
	ON_EVENT(CEditNetworkID, IDC_COMBO_PROVIDERS, 16 /* SelChosen */, OnSelChosenComboProviders, VTS_I4)
	ON_EVENT(CEditNetworkID, IDC_COMBO_PROVIDERS, 18 /* RequeryFinished */, OnRequeryFinishedComboProviders, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditNetworkID::OnSelChosenComboProviders(long nRow) 
{
	if(nRow==-1)
		return;
	m_iProviderID = m_ComboProviders->GetValue(nRow,0).lVal;
	LoadNetworkID();	
}

void CEditNetworkID::OnRequeryFinishedComboProviders(short nFlags) 
{
	m_ComboProviders->CurSel = 0;

	if(m_ComboProviders->CurSel != -1) {
		m_iProviderID = m_ComboProviders->GetValue(m_ComboProviders->GetCurSel(),0).lVal;
	}

	LoadNetworkID();
}

void CEditNetworkID::OnKillfocusEditNetworkid() 
{
	SaveNetworkID();	
}

void CEditNetworkID::OnAdvNetworkidEdit() 
{
	CAdvInsuranceGRPEdit dlg(this);
	dlg.m_IDType = 3;
	dlg.DoModal();

	LoadNetworkID();
}

void CEditNetworkID::SaveNetworkID()
{
	if(m_ComboProviders->CurSel==-1)
		return;

	CString str;
	try {
		//for auditing
		CString strOld;
		_RecordsetPtr rs = CreateRecordset("SELECT NetworkID FROM InsuranceNetworkID WHERE InsCoID = %d AND ProviderID = %d", m_iInsuranceCoID, m_iProviderID);
		if(!rs->eof && rs->Fields->Item["NetworkID"]->Value.vt == VT_BSTR) {
			strOld = CString(rs->Fields->Item["NetworkID"]->Value.bstrVal);
		}
		UpdateData(TRUE);

		if(m_NetworkID != "") {
			str.Format("UPDATE InsuranceNetworkID SET NetworkID = '%s' WHERE InsCoID = %d AND ProviderID = %d",
				_Q(m_NetworkID), m_iInsuranceCoID, m_iProviderID);
			
			// (j.jones 2010-04-19 16:16) - PLID 30852 - corrected potential formatting issues by simply parameterizing
			ExecuteParamSql("IF EXISTS (SELECT NetworkID FROM InsuranceNetworkID WHERE InsCoID = {INT} AND ProviderID = {INT}) \r\n"
				"BEGIN \r\n"
				"	UPDATE InsuranceNetworkID \r\n"
				"	SET NetworkID = {STRING} WHERE InsCoID = {INT} AND ProviderID = {INT} \r\n"
				"END \r\n"
				"ELSE BEGIN \r\n"
				"	INSERT INTO InsuranceNetworkID (InsCoID, ProviderID, NetworkID) VALUES ({INT}, {INT}, {STRING}) \r\n"
				"END \r\n",
				m_iInsuranceCoID, m_iProviderID,
				m_NetworkID, m_iInsuranceCoID, m_iProviderID,
				m_iInsuranceCoID, m_iProviderID, m_NetworkID);
		}
		else {
			//if empty, remove the record
			ExecuteParamSql("DELETE FROM InsuranceNetworkID WHERE InsCoID = {INT} AND ProviderID = {INT}",m_iInsuranceCoID,m_iProviderID);
		}
		//auditing
		CString strPerson;
		strPerson = CString(m_ComboProviders->GetValue(m_ComboProviders->CurSel, 1).bstrVal) + " / " + m_strInsCo;
		if(strOld != m_NetworkID) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, strPerson, nAuditID, aeiInsCoNetworkID, -1, strOld, m_NetworkID, aepMedium, aetChanged);
		}

	} NxCatchAll("Error in SaveNetworkID");
}

void CEditNetworkID::LoadNetworkID()
{
	_RecordsetPtr rs;
	_variant_t var;
	CString str;

	if (m_ComboProviders->GetRowCount() == 0) {
		m_NetworkID.Empty();
	}
	else try {

		str.Format("SELECT NetworkID FROM InsuranceNetworkID WHERE InsCoID = %d AND ProviderID = %d",
			m_iInsuranceCoID, m_iProviderID);
		rs = CreateRecordset(str);
		if (!rs->eof) {
			var = rs->Fields->GetItem("NetworkID")->Value;
			if (var.vt == VT_NULL)
				m_NetworkID.Empty();
			else
				m_NetworkID = CString(var.bstrVal);
		}
		else
			m_NetworkID.Empty();
		rs->Close();
	}
	NxCatchAll("Error in LoadNetworkID");

	UpdateData(FALSE);
}

void CEditNetworkID::OnOK() 
{
	SaveNetworkID();

	CDialog::OnOK();
}
