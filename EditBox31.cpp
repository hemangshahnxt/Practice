// EditBox31.cpp : implementation file
//

#include "stdafx.h"
#include "pracprops.h"
#include "AdvInsuranceGRPEdit.h"
#include "EditBox31.h"
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
// CEditBox31 dialog


CEditBox31::CEditBox31(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditBox31::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditBox31)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditBox31::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditBox31)
	DDX_Text(pDX, IDC_EDIT_BOX31, m_Box31);
	DDV_MaxChars(pDX, m_Box31, 15);
	DDX_Control(pDX, IDC_EDIT_BOX31, m_nxeditEditBox31);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ADV_BOX31_EDIT, m_btnAdvBox31Edit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditBox31, CNxDialog)
	//{{AFX_MSG_MAP(CEditBox31)
	ON_BN_CLICKED(IDC_ADV_BOX31_EDIT, OnAdvBox31Edit)
	ON_EN_KILLFOCUS(IDC_EDIT_BOX31, OnKillfocusEditBox31)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditBox31 message handlers

BOOL CEditBox31::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-07 11:05) - PLID 29817 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnAdvBox31Edit.AutoSet(NXB_MODIFY);

		m_ComboProviders = BindNxDataListCtrl(this,IDC_COMBO_PROVIDERS,GetRemoteData(),true);
	}
	NxCatchAll("Error in CEditBox31::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_EVENTSINK_MAP(CEditBox31, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditBox31)
	ON_EVENT(CEditBox31, IDC_COMBO_PROVIDERS, 16 /* SelChosen */, OnSelChosenComboProviders, VTS_I4)
	ON_EVENT(CEditBox31, IDC_COMBO_PROVIDERS, 18 /* RequeryFinished */, OnRequeryFinishedComboProviders, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditBox31::OnSelChosenComboProviders(long nRow) 
{
	if(nRow==-1)
		return;
	m_iProviderID = m_ComboProviders->GetValue(nRow,0).lVal;
	LoadBox31Info();
}

void CEditBox31::OnOK() 
{
	SaveBox31Info();
	
	CDialog::OnOK();
}

void CEditBox31::OnAdvBox31Edit() 
{
	CAdvInsuranceGRPEdit dlg(this);
	dlg.m_IDType = 5;
	dlg.DoModal();

	LoadBox31Info();
}

void CEditBox31::OnKillfocusEditBox31() 
{
	SaveBox31Info();	
}

void CEditBox31::LoadBox31Info()
{
	
	_RecordsetPtr rs;
	_variant_t var;
	CString str;

	if (m_ComboProviders->GetRowCount() == 0) {
		m_Box31.Empty();
	}
	else try {

		str.Format("SELECT Box31Info FROM InsuranceBox31 WHERE InsCoID = %d AND ProviderID = %d",
			m_iInsuranceCoID, m_iProviderID);
		rs = CreateRecordset(str);
		if (!rs->eof) {
			var = rs->Fields->GetItem("Box31Info")->Value;
			if (var.vt == VT_NULL)
				m_Box31.Empty();
			else
				m_Box31 = CString(var.bstrVal);
		}
		else
			m_Box31.Empty();
		rs->Close();
	}
	NxCatchAll("Error in LoadBox31Info");

	UpdateData(FALSE);
}

void CEditBox31::SaveBox31Info()
{
	if(m_ComboProviders->CurSel==-1)
		return;

	//for auditing
	CString strOld;
	_RecordsetPtr rs = CreateRecordset("SELECT Box31Info FROM InsuranceBox31 WHERE InsCoID = %d AND ProviderID = %d", m_iInsuranceCoID, m_iProviderID);
	if(!rs->eof && rs->Fields->Item["Box31Info"]->Value.vt == VT_BSTR) {
		strOld = CString(rs->Fields->Item["Box31Info"]->Value.bstrVal);
	}

	CString str;
	try {
		UpdateData(TRUE);

		if(m_Box31 != "") {
			// (j.jones 2010-04-19 17:20) - PLID 30852 - corrected potential formatting issues by simply parameterizing
			ExecuteParamSql("IF EXISTS (SELECT Box31Info FROM InsuranceBox31 WHERE InsCoID = {INT} AND ProviderID = {INT}) \r\n"
				"BEGIN \r\n"
				"	UPDATE InsuranceBox31 \r\n"
				"	SET Box31Info = {STRING} WHERE InsCoID = {INT} AND ProviderID = {INT} \r\n"
				"END \r\n"
				"ELSE BEGIN \r\n"
				"	INSERT INTO InsuranceBox31 (InsCoID, ProviderID, Box31Info) VALUES ({INT}, {INT}, {STRING}) \r\n"
				"END \r\n",
				m_iInsuranceCoID, m_iProviderID,
				m_Box31, m_iInsuranceCoID, m_iProviderID,
				m_iInsuranceCoID, m_iProviderID, m_Box31);
		}
		else {
			//if empty, remove the record
			ExecuteParamSql("DELETE FROM InsuranceBox31 WHERE InsCoID = {INT} AND ProviderID = {INT}",m_iInsuranceCoID,m_iProviderID);
		}

		//auditing
		CString strPerson;
		strPerson = CString(m_ComboProviders->GetValue(m_ComboProviders->CurSel, 1).bstrVal) + " / " + m_strInsCo;
		if(strOld != m_Box31) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, strPerson, nAuditID, aeiInsCoHCFABox31, -1, strOld, m_Box31, aepMedium, aetChanged);
		}

	} NxCatchAll("Error in SaveBox31Info");
}

void CEditBox31::OnRequeryFinishedComboProviders(short nFlags) 
{
	m_ComboProviders->CurSel = 0;

	if(m_ComboProviders->CurSel != -1) {
		m_iProviderID = m_ComboProviders->GetValue(m_ComboProviders->GetCurSel(),0).lVal;
	}

	LoadBox31Info();	
}
