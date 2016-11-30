// EditBox24J.cpp : Adds user-entered data to HCFA Box 24J, per provider, per ins.co.
//

#include "stdafx.h"
#include "practice.h"
#include "pracprops.h"
#include "EditBox24J.h"
#include "AdvInsuranceGRPEdit.h"
#include "AuditTrail.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (j.jones 2006-09-14 15:21) - PLID 22424 - converted this dialog from EditBox24K

/////////////////////////////////////////////////////////////////////////////
// CEditBox24J dialog


CEditBox24J::CEditBox24J(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditBox24J::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditBox24J)
	m_Box24J = _T("");
	m_Box24IQual = _T("");
	//}}AFX_DATA_INIT
}


void CEditBox24J::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditBox24J)
	DDX_Text(pDX, IDC_EDIT_BOX24J_NUM, m_Box24J);
	DDV_MaxChars(pDX, m_Box24J, 50);
	DDX_Text(pDX, IDC_EDIT_BOX24I_QUAL, m_Box24IQual);
	DDV_MaxChars(pDX, m_Box24IQual, 10);
	DDX_Control(pDX, IDC_EDIT_BOX24I_QUAL, m_nxeditEditBox24iQual);
	DDX_Control(pDX, IDC_EDIT_BOX24J_NUM, m_nxeditEditBox24jNum);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ADV_BOX24J_EDIT, m_btnAdvBox24JEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditBox24J, CNxDialog)
	//{{AFX_MSG_MAP(CEditBox24J)
	ON_EN_KILLFOCUS(IDC_EDIT_BOX24J_NUM, OnKillFocusEditBox24J)
	ON_BN_CLICKED(IDC_ADV_BOX24J_EDIT, OnAdvBox24JEdit)
	ON_EN_KILLFOCUS(IDC_EDIT_BOX24I_QUAL, OnKillfocusEditBox24iQual)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditBox24J message handlers

BOOL CEditBox24J::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-07 11:02) - PLID 29817 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnAdvBox24JEdit.AutoSet(NXB_MODIFY);

		m_ComboProviders = BindNxDataListCtrl(this,IDC_COMBO_PROVIDERS,GetRemoteData(),true);
	}
	NxCatchAll("Error in CEditBox24J::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditBox24J::LoadBox24Info()
{
	
	_RecordsetPtr rs;
	_variant_t var;
	CString str;

	if (m_ComboProviders->GetRowCount() == 0) {
		m_Box24IQual.Empty();
		m_Box24J.Empty();		
	}
	else try {

		str.Format("SELECT Box24IQualifier, Box24JNumber FROM InsuranceBox24J WHERE InsCoID = %d AND ProviderID = %d",
			m_iInsuranceCoID, m_iProviderID);
		rs = CreateRecordset(str);
		if (!rs->eof) {
			var = rs->Fields->GetItem("Box24IQualifier")->Value;
			if (var.vt == VT_NULL)
				m_Box24IQual.Empty();
			else
				m_Box24IQual = CString(var.bstrVal);
			var = rs->Fields->GetItem("Box24JNumber")->Value;
			if (var.vt == VT_NULL)
				m_Box24J.Empty();
			else
				m_Box24J = CString(var.bstrVal);
		}
		else {
			m_Box24IQual.Empty();
			m_Box24J.Empty();
		}
		rs->Close();
	}
	NxCatchAll("Error in LoadBox24Info");

	UpdateData(FALSE);
}

void CEditBox24J::SaveBox24Info()
{
	if(m_ComboProviders->CurSel==-1)
		return;

	CString str;
	try {
		//for auditing
		CString strOldNum, strOldQual;
		_RecordsetPtr rs = CreateRecordset("SELECT Box24IQualifier, Box24JNumber FROM InsuranceBox24J WHERE InsCoID = %d AND ProviderID = %d", m_iInsuranceCoID, m_iProviderID);
		if(!rs->eof) {
			strOldQual = AdoFldString(rs, "Box24IQualifier","");
			strOldNum = AdoFldString(rs, "Box24JNumber","");
		}

		UpdateData(TRUE);

		if(m_Box24J != "" || m_Box24IQual != "") {
			// (j.jones 2010-04-19 17:20) - PLID 30852 - corrected potential formatting issues by simply parameterizing
			ExecuteParamSql("IF EXISTS (SELECT Box24JNumber FROM InsuranceBox24J WHERE InsCoID = {INT} AND ProviderID = {INT}) \r\n"
				"BEGIN \r\n"
				"	UPDATE InsuranceBox24J \r\n"
				"	SET Box24JNumber = {STRING}, Box24IQualifier = {STRING} WHERE InsCoID = {INT} AND ProviderID = {INT} \r\n"
				"END \r\n"
				"ELSE BEGIN \r\n"
				"	INSERT INTO InsuranceBox24J (InsCoID, ProviderID, Box24JNumber, Box24IQualifier) VALUES ({INT}, {INT}, {STRING}, {STRING}) \r\n"
				"END \r\n",
				m_iInsuranceCoID, m_iProviderID,
				m_Box24J, m_Box24IQual, m_iInsuranceCoID, m_iProviderID,
				m_iInsuranceCoID, m_iProviderID, m_Box24J, m_Box24IQual);
		}
		else {
			//if empty, remove the record
			ExecuteParamSql("DELETE FROM InsuranceBox24J WHERE InsCoID = {INT} AND ProviderID = {INT}",m_iInsuranceCoID,m_iProviderID);
		}

		//auditing
		CString strPerson;
		strPerson = CString(m_ComboProviders->GetValue(m_ComboProviders->CurSel, 1).bstrVal) + " / " + m_strInsCo;
		if(strOldNum != m_Box24J || strOldQual != m_Box24IQual) {

			CString strOld, strNew;
			strOld.Format("%s %s", strOldQual, strOldNum);
			strNew.Format("%s %s", m_Box24IQual, m_Box24J);

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, strPerson, nAuditID, aeiInsCoHCFA24J, -1, strOld, strNew, aepMedium, aetChanged);
		}

	} NxCatchAll("Error in SaveBox24Info");
}

BEGIN_EVENTSINK_MAP(CEditBox24J, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditbox24J)
	ON_EVENT(CEditBox24J, IDC_COMBO_PROVIDERS, 2 /* SelectionChange */, OnSelectionChangeComboProviders, VTS_I4)
	ON_EVENT(CEditBox24J, IDC_COMBO_PROVIDERS, 18 /* RequeryFinished */, OnRequeryFinishedComboProviders, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CEditBox24J::OnSelectionChangeComboProviders(long iNewRow) 
{
	if(iNewRow==-1)
		return;
	m_iProviderID = m_ComboProviders->GetValue(iNewRow,0).lVal;
	LoadBox24Info();
}

void CEditBox24J::OnOK() 
{
	SaveBox24Info();
	
	CDialog::OnOK();
}

void CEditBox24J::OnKillFocusEditBox24J() 
{
	SaveBox24Info();
}

void CEditBox24J::OnRequeryFinishedComboProviders(short nFlags) 
{
	m_ComboProviders->CurSel = 0;

	if(m_ComboProviders->CurSel != -1) {
		m_iProviderID = m_ComboProviders->GetValue(m_ComboProviders->GetCurSel(),0).lVal;
	}

	LoadBox24Info();	
}

void CEditBox24J::OnAdvBox24JEdit() 
{
	CAdvInsuranceGRPEdit dlg(this);
	dlg.m_IDType = 2;
	dlg.DoModal();

	LoadBox24Info();
}

void CEditBox24J::OnKillfocusEditBox24iQual() 
{
	SaveBox24Info();
}
