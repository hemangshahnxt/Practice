// EditFacilityID.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "pracprops.h"
#include "EditFacilityID.h"
#include "AuditTrail.h"
#include "AdvFacilityIDEditDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace ADODB;

// (a.walling 2010-01-21 16:43) - PLID 37022 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CEditFacilityID dialog


CEditFacilityID::CEditFacilityID(CWnd* pParent /*=NULL*/)
	: CNxDialog(CEditFacilityID::IDD, pParent)
{
	//{{AFX_DATA_INIT(CEditFacilityID)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditFacilityID::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditFacilityID)
	DDX_Text(pDX, IDC_EDIT_FACILITY_ID, m_FacilityID);
	DDV_MaxChars(pDX, m_FacilityID, 20);
	DDX_Text(pDX, IDC_EDIT_FACILITY_ID_QUAL, m_Qualifier);
	DDV_MaxChars(pDX, m_Qualifier, 10);
	DDX_Control(pDX, IDC_EDIT_FACILITY_ID, m_nxeditEditFacilityId);
	DDX_Control(pDX, IDC_EDIT_FACILITY_ID_QUAL, m_nxeditEditFacilityIdQual);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_ADV_FACILITY_ID_EDIT, m_btnAdvFacilityIDEdit);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CEditFacilityID, CNxDialog)
	//{{AFX_MSG_MAP(CEditFacilityID)
	ON_EN_KILLFOCUS(IDC_EDIT_FACILITY_ID, OnKillfocusEditFacilityId)
	ON_EN_KILLFOCUS(IDC_EDIT_FACILITY_ID_QUAL, OnKillfocusEditFacilityIdQual)
	ON_BN_CLICKED(IDC_ADV_FACILITY_ID_EDIT, OnAdvFacilityIdEdit)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CEditFacilityID message handlers

BEGIN_EVENTSINK_MAP(CEditFacilityID, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CEditFacilityID)
	ON_EVENT(CEditFacilityID, IDC_COMBO_LOCATIONS, 16 /* SelChosen */, OnSelChosenComboLocations, VTS_I4)
	ON_EVENT(CEditFacilityID, IDC_COMBO_LOCATIONS, 18 /* RequeryFinished */, OnRequeryFinishedComboLocations, VTS_I2)
	ON_EVENT(CEditFacilityID, IDC_COMBO_LOCATIONS, 1 /* SelChanging */, OnSelChangingComboLocations, VTS_PI4)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

BOOL CEditFacilityID::OnInitDialog() 
{
	try {
		CNxDialog::OnInitDialog();
		
		// (c.haag 2008-05-07 11:02) - PLID 29817 - NxIconify the buttons
		m_btnOK.AutoSet(NXB_CLOSE);
		m_btnAdvFacilityIDEdit.AutoSet(NXB_MODIFY);

		m_ComboLocations = BindNxDataListCtrl(this,IDC_COMBO_LOCATIONS,GetRemoteData(),true);
	}
	NxCatchAll("Error in CEditFacilityID::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CEditFacilityID::OnSelChosenComboLocations(long nRow) 
{
	if(nRow==-1)
		return;
	m_iLocationID = m_ComboLocations->GetValue(nRow,0).lVal;
	LoadFacilityID();
}

void CEditFacilityID::OnRequeryFinishedComboLocations(short nFlags) 
{
	m_ComboLocations->CurSel = 0;

	if(m_ComboLocations->CurSel != -1) {
		m_iLocationID = m_ComboLocations->GetValue(m_ComboLocations->GetCurSel(),0).lVal;
	}

	LoadFacilityID();	
}

void CEditFacilityID::OnKillfocusEditFacilityId() 
{
// (a.walling 2006-11-09 17:09) - PLID 23370 - Save is handled when closing the dialog or choosing a different location
//	SaveFacilityID();
}

void CEditFacilityID::OnKillfocusEditFacilityIdQual() 
{
// (a.walling 2006-11-09 17:09) - PLID 23370 - Save is handled when closing the dialog or choosing a different location
//	SaveFacilityID();	
}

void CEditFacilityID::OnOK() 
{
	SaveFacilityID();
	
	CDialog::OnOK();
}

void CEditFacilityID::LoadFacilityID()
{
	
	_RecordsetPtr rs;
	_variant_t var;
	CString str;

	if (m_ComboLocations->GetRowCount() == 0) {
		m_FacilityID.Empty();
		m_Qualifier.Empty();
	}
	else try {

		str.Format("SELECT FacilityID, Qualifier FROM InsuranceFacilityID WHERE InsCoID = %d AND LocationID = %d",
			m_iInsuranceCoID, m_iLocationID);
		rs = CreateRecordset(str);
		if (!rs->eof) {
			var = rs->Fields->GetItem("FacilityID")->Value;
			if (var.vt == VT_NULL)
				m_FacilityID.Empty();
			else
				m_FacilityID = CString(var.bstrVal);
			var = rs->Fields->GetItem("Qualifier")->Value;
			if (var.vt == VT_NULL)
				m_Qualifier.Empty();
			else
				m_Qualifier = CString(var.bstrVal);
		}
		else {
			m_FacilityID.Empty();
			m_Qualifier.Empty();
		}
		rs->Close();
	}
	NxCatchAll("Error in LoadFacilityID");

	UpdateData(FALSE);
}

void CEditFacilityID::SaveFacilityID()
{
	if(m_ComboLocations->CurSel==-1)
		return;

	CString str;
	try {
		//for auditing
		CString strOldID, strOldQual;
		_RecordsetPtr rs = CreateRecordset("SELECT FacilityID, Qualifier FROM InsuranceFacilityID WHERE InsCoID = %d AND LocationID = %d", m_iInsuranceCoID, m_iLocationID);
		if(!rs->eof && rs->Fields->Item["FacilityID"]->Value.vt == VT_BSTR) {
			strOldID = AdoFldString(rs, "FacilityID", "");
			strOldQual = AdoFldString(rs, "Qualifier", "");
		}
		UpdateData(TRUE);
		
		if(m_FacilityID != "" || m_Qualifier != "") {
			// (j.jones 2010-04-19 17:20) - PLID 30852 - corrected potential formatting issues by simply parameterizing
			ExecuteParamSql("IF EXISTS (SELECT FacilityID FROM InsuranceFacilityID WHERE InsCoID = {INT} AND LocationID = {INT}) \r\n"
				"BEGIN \r\n"
				"	UPDATE InsuranceFacilityID \r\n"
				"	SET FacilityID = {STRING}, Qualifier = {STRING} WHERE InsCoID = {INT} AND LocationID = {INT} \r\n"
				"END \r\n"
				"ELSE BEGIN \r\n"
				"	INSERT INTO InsuranceFacilityID (InsCoID, LocationID, FacilityID, Qualifier) VALUES ({INT}, {INT}, {STRING}, {STRING}) \r\n"
				"END \r\n",
				m_iInsuranceCoID, m_iLocationID,
				m_FacilityID, m_Qualifier, m_iInsuranceCoID, m_iLocationID,
				m_iInsuranceCoID, m_iLocationID, m_FacilityID, m_Qualifier);
		}
		else {
			//if empty, remove the record
			ExecuteParamSql("DELETE FROM InsuranceFacilityID WHERE InsCoID = {INT} AND LocationID = {INT}",m_iInsuranceCoID,m_iLocationID);
		}

		//auditing
		CString strPerson;
		strPerson = CString(m_ComboLocations->GetValue(m_ComboLocations->CurSel, 1).bstrVal) + " / " + m_strInsCo;
		if(strOldID != m_FacilityID || strOldQual != m_Qualifier) {

			CString strOld, strNew;
			strOld.Format("%s %s", strOldQual, strOldID);
			strNew.Format("%s %s", m_Qualifier, m_FacilityID);

			long nAuditID = BeginNewAuditEvent();
			AuditEvent(-1, strPerson, nAuditID, aeiInsCoFacilityID, -1, strOld, strNew, aepMedium, aetChanged);
		}

	} NxCatchAll("Error in SaveFacilityID");
}

void CEditFacilityID::OnAdvFacilityIdEdit() 
{
	SaveFacilityID();

	CAdvFacilityIDEditDlg dlg(this);
	dlg.DoModal();

	LoadFacilityID();
}

void CEditFacilityID::OnSelChangingComboLocations(long FAR* nNewSel) 
{
	// (a.walling 2006-11-09 17:09) - PLID 23370 - Save is handled when closing the dialog or choosing a different location
	// this prevents an infinite loops caused by KillFocus events when entering too much text.
	SaveFacilityID();
}
