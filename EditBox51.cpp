// EditBox51.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "pracprops.h"
#include "AdvInsuranceGRPEdit.h"
#include "EditBox51.h"
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
// CEditBox51 dialog


CEditBox51::CEditBox51(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditBox51::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditBox51)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditBox51::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditBox51)
	DDX_Text(pDX, IDC_EDIT_BOX51, m_Box51);
	DDV_MaxChars(pDX, m_Box51, 15);
	DDX_Control(pDX, IDC_EDIT_BOX51, m_nxeditEditBox51);
	DDX_Control(pDX, IDC_BOX_51_LABEL, m_nxstaticBox51Label);
	DDX_Control(pDX, IDC_BOX_51_CAPTION, m_nxstaticBox51Caption);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ADV_BOX51_EDIT, m_btnAdvBox51Edit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditBox51, CNxDialog)
	//{{AFX_MSG_MAP(CEditBox51)
	ON_BN_CLICKED(IDC_ADV_BOX51_EDIT, OnAdvBox51Edit)
	ON_EN_KILLFOCUS(IDC_EDIT_BOX51, OnKillfocusEditBox51)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditBox51 message handlers

BEGIN_EVENTSINK_MAP(CEditBox51, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditBox51)
	ON_EVENT(CEditBox51, IDC_COMBO_PROVIDERS, 16 /* SelChosen */, OnSelChosenComboProviders, VTS_I4)
	ON_EVENT(CEditBox51, IDC_COMBO_PROVIDERS, 18 /* RequeryFinished */, OnRequeryFinishedComboProviders, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CEditBox51::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();

		// (c.haag 2008-05-07 11:04) - PLID 29817 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnAdvBox51Edit.AutoSet(NXB_MODIFY);

		m_ComboProviders = BindNxDataListCtrl(this,IDC_COMBO_PROVIDERS,GetRemoteData(),true);
		
		//TES 3/12/2007 - PLID 25295 - On the UB04, we'll call this the Other Prv ID, because it can be in multiple boxes.
		if(GetUBFormType() == eUB04) {//UB04
			SetWindowText("Edit Other Prv ID");
			SetDlgItemText(IDC_BOX_51_CAPTION, "This feature will allow you to fill a default UB Other Provider ID\n(for use in Boxes 51, 57, and 76-79) for a provider and ins. co.");
			SetDlgItemText(IDC_BOX_51_LABEL, "Other Prv ID");
		}
		else {//UB92
			SetWindowText("Edit UB92 Box 51");
			SetDlgItemText(IDC_BOX_51_CAPTION, "This feature will allow you to fill UB92 Box 51\nfor a provider and insurance company.");
			SetDlgItemText(IDC_BOX_51_LABEL, "UB92 Box 51");
		}
	}
	NxCatchAll("Error in CEditBox51::OnInitDialog");

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditBox51::OnSelChosenComboProviders(long nRow) 
{
	if(nRow==-1)
		return;
	m_iProviderID = m_ComboProviders->GetValue(nRow,0).lVal;
	LoadBox51Info();
}

void CEditBox51::OnAdvBox51Edit() 
{
	CAdvInsuranceGRPEdit dlg(this);
	dlg.m_IDType = 4;
	dlg.DoModal();

	LoadBox51Info();
}

void CEditBox51::OnOK() 
{
	SaveBox51Info();
	
	CDialog::OnOK();
}

void CEditBox51::OnKillfocusEditBox51() 
{
	SaveBox51Info();	
}

void CEditBox51::LoadBox51Info()
{
	
	_RecordsetPtr rs;
	_variant_t var;
	CString str;

	if (m_ComboProviders->GetRowCount() == 0) {
		m_Box51.Empty();
	}
	else try {

		str.Format("SELECT Box51Info FROM InsuranceBox51 WHERE InsCoID = %d AND ProviderID = %d",
			m_iInsuranceCoID, m_iProviderID);
		rs = CreateRecordset(str);
		if (!rs->eof) {
			var = rs->Fields->GetItem("Box51Info")->Value;
			if (var.vt == VT_NULL)
				m_Box51.Empty();
			else
				m_Box51 = CString(var.bstrVal);
		}
		else
			m_Box51.Empty();
		rs->Close();
	}
	NxCatchAll("Error in LoadBox51Info");

	UpdateData(FALSE);
}

void CEditBox51::SaveBox51Info()
{
	if(m_ComboProviders->CurSel==-1)
		return;

	//for auditing
	CString strOld;
	_RecordsetPtr rs = CreateRecordset("SELECT Box51Info FROM InsuranceBox51 WHERE InsCoID = %d AND ProviderID = %d", m_iInsuranceCoID, m_iProviderID);
	if(!rs->eof && rs->Fields->Item["Box51Info"]->Value.vt == VT_BSTR) {
		strOld = CString(rs->Fields->Item["Box51Info"]->Value.bstrVal);
	}

	CString str;
	try {
		UpdateData(TRUE);

		if(m_Box51 != "") {
			// (j.jones 2010-04-19 16:16) - PLID 30852 - corrected potential formatting issues by simply parameterizing
			ExecuteParamSql("IF EXISTS (SELECT Box51Info FROM InsuranceBox51 WHERE InsCoID = {INT} AND ProviderID = {INT}) \r\n"
				"BEGIN \r\n"
				"	UPDATE InsuranceBox51 \r\n"
				"	SET Box51Info = {STRING} WHERE InsCoID = {INT} AND ProviderID = {INT} \r\n"
				"END \r\n"
				"ELSE BEGIN \r\n"
				"	INSERT INTO InsuranceBox51 (InsCoID, ProviderID, Box51Info) VALUES ({INT}, {INT}, {STRING}) \r\n"
				"END \r\n",
				m_iInsuranceCoID, m_iProviderID,
				m_Box51, m_iInsuranceCoID, m_iProviderID,
				m_iInsuranceCoID, m_iProviderID, m_Box51);
		}
		else {
			//if empty, remove the record
			ExecuteParamSql("DELETE FROM InsuranceBox51 WHERE InsCoID = {INT} AND ProviderID = {INT}",m_iInsuranceCoID,m_iProviderID);
		}

		//auditing
		CString strPerson;
		strPerson = CString(m_ComboProviders->GetValue(m_ComboProviders->CurSel, 1).bstrVal) + " / " + m_strInsCo;
		if(strOld != m_Box51) {
			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, strPerson, nAuditID, aeiInsCoUB92Box51, -1, strOld, m_Box51, aepMedium, aetChanged);
		}

	} NxCatchAll("Error in SaveBox51Info");
}

void CEditBox51::OnRequeryFinishedComboProviders(short nFlags) 
{
	m_ComboProviders->CurSel = 0;

	if(m_ComboProviders->CurSel != -1) {
		m_iProviderID = m_ComboProviders->GetValue(m_ComboProviders->GetCurSel(),0).lVal;
	}

	LoadBox51Info();	
}
