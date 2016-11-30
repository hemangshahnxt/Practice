// AdvHCFAPinSetup.cpp : implementation file
//

#include "stdafx.h"
#include "AdvHCFAPinSetup.h"
#include "GlobalDrawingUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;

/////////////////////////////////////////////////////////////////////////////
// CAdvHCFAPinSetup dialog


CAdvHCFAPinSetup::CAdvHCFAPinSetup(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdvHCFAPinSetup::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAdvHCFAPinSetup)
		m_ProviderID = -1;
		m_LocationID = -1;
		m_bLocationsLoaded = FALSE;
		m_bProvidersLoaded = FALSE;
		m_bHas33bQualChanged = FALSE;
	//}}AFX_DATA_INIT
}


void CAdvHCFAPinSetup::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAdvHCFAPinSetup)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	DDX_Control(pDX, IDC_RADIO_OVERRIDE_SSN, m_radioSSN);
	DDX_Control(pDX, IDC_RADIO_OVERRIDE_EIN, m_radioEIN);
	DDX_Control(pDX, IDC_RADIO_OVERRIDE_NONE, m_radioNoOverride);
	DDX_Control(pDX, IDC_ADV_BOX33_NAME, m_nxeditAdvBox33Name);
	DDX_Control(pDX, IDC_ADV_BOX19, m_nxeditAdvBox19);
	DDX_Control(pDX, IDC_BOX24J_QUAL, m_nxeditBox24jQual);
	DDX_Control(pDX, IDC_ADV_BOX24J, m_nxeditAdvBox24j);
	DDX_Control(pDX, IDC_ADV_BOX24J_NPI, m_nxeditAdvBox24jNpi);
	DDX_Control(pDX, IDC_ADV_EIN, m_nxeditAdvEin);
	DDX_Control(pDX, IDC_ADV_BOX33A_NPI, m_nxeditAdvBox33aNpi);
	DDX_Control(pDX, IDC_BOX33_QUAL, m_nxeditBox33Qual);
	DDX_Control(pDX, IDC_PIN, m_nxeditPin);
	DDX_Control(pDX, IDC_ADV_GRP, m_nxeditAdvGrp);
	DDX_Control(pDX, IDC_HCFA_GROUP_NAME, m_nxstaticHcfaGroupName);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_ADV_BOX33_ADDRESS1, m_nxeditBox33Address1);
	DDX_Control(pDX, IDC_ADV_BOX33_ADDRESS2, m_nxeditBox33Address2);
	DDX_Control(pDX, IDC_ADV_BOX33_CITY, m_nxeditBox33City);
	DDX_Control(pDX, IDC_ADV_BOX33_STATE, m_nxeditBox33State);
	DDX_Control(pDX, IDC_ADV_BOX33_ZIP, m_nxeditBox33Zip);
	DDX_Control(pDX, IDC_GRP_LABEL, m_nxstaticGRPLabel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAdvHCFAPinSetup, CNxDialog)
	//{{AFX_MSG_MAP(CAdvHCFAPinSetup)
	ON_EN_KILLFOCUS(IDC_ADV_BOX33_ZIP, OnKillfocusZipBox) // (b.eyers 2015-04-09) - PLID 59169 
	ON_EN_KILLFOCUS(IDC_ADV_BOX33_CITY, OnKillfocusCityBox) // (b.eyers 2015-04-09) - PLID 59169
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAdvHCFAPinSetup message handlers

BOOL CAdvHCFAPinSetup::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (z.manning, 04/30/2008) - PLID 29850 - Set button styles
	m_btnOk.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);	
	
	m_brush.CreateSolidBrush(PaletteColor(GetNxColor(GNC_ADMIN, 0)));

	m_ProvidersCombo = BindNxDataListCtrl(this,IDC_PROVIDERS_COMBO,GetRemoteData(),true);
	m_LocationsCombo = BindNxDataListCtrl(this,IDC_LOCATIONS_COMBO,GetRemoteData(),true);

	_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM HCFASetupT WHERE ID = {INT}",m_HCFAGroup);

	if(!rs->eof) {
		CString name = CString(rs->Fields->Item["Name"]->Value.bstrVal);
		SetDlgItemText(IDC_HCFA_GROUP_NAME,name);
	}
	rs->Close();

	// (j.jones 2011-09-23 17:36) - PLID 28441 - hide the group box unless it is filled for at least
	// one setup in the data - in other words, you can't see the edit box unless you have used it before
	// (in the rare case they have a non-empty blank string, assume it's filled)
	if(!ReturnsRecordsParam("SELECT GRP FROM AdvHCFAPinT WHERE GRP Is Not Null AND GRP <> ''")) {
		GetDlgItem(IDC_GRP_LABEL)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_ADV_GRP)->ShowWindow(SW_HIDE);
	}

	((CNxEdit*)GetDlgItem(IDC_ADV_BOX19))->SetLimitText(50);
	((CNxEdit*)GetDlgItem(IDC_BOX24J_QUAL))->SetLimitText(10);
	((CNxEdit*)GetDlgItem(IDC_ADV_BOX24J))->SetLimitText(50);
	((CNxEdit*)GetDlgItem(IDC_ADV_EIN))->SetLimitText(50);
	((CNxEdit*)GetDlgItem(IDC_BOX33_QUAL))->SetLimitText(10);
	((CNxEdit*)GetDlgItem(IDC_PIN))->SetLimitText(50);
	// (j.jones 2007-02-26 09:16) - PLID 24908 - re-added GRP box
	((CNxEdit*)GetDlgItem(IDC_ADV_GRP))->SetLimitText(50);
	// (j.jones 2007-05-07 13:53) - PLID 25922 - added Box 33 Name override
	((CNxEdit*)GetDlgItem(IDC_ADV_BOX33_NAME))->SetLimitText(50);
	// (j.jones 2007-08-08 10:04) - PLID 25395 - added the NPI fields
	((CNxEdit*)GetDlgItem(IDC_ADV_BOX24J_NPI))->SetLimitText(50);
	((CNxEdit*)GetDlgItem(IDC_ADV_BOX33A_NPI))->SetLimitText(50);
	// (j.jones 2010-02-01 09:31) - PLID 37137 - added the Box 33 address override
	m_nxeditBox33Address1.SetLimitText(75);
	m_nxeditBox33Address2.SetLimitText(75);
	m_nxeditBox33City.SetLimitText(50);
	m_nxeditBox33State.SetLimitText(20);
	m_nxeditBox33Zip.SetLimitText(20);

	// (b.eyers 2015-04-09) - PLID 59169 
	g_propManager.CachePropertiesInBulk("AdvHCFAPinSetup", propNumber,
		"(Username = '<None>' OR Username = '%s') AND Name IN ( "
		"	'LookupZipStateByCity' "
		")", _Q(GetCurrentUserName()));

	m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");

	if (m_bLookupByCity) {
		ChangeZOrder(IDC_ADV_BOX33_ZIP, IDC_ADV_BOX33_STATE);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CAdvHCFAPinSetup::OnOK() 
{
	// (a.walling 2008-05-23 14:32) - PLID 27810 - Cancelable
	if (!Save())
		return;
	
	CDialog::OnOK();
}

BEGIN_EVENTSINK_MAP(CAdvHCFAPinSetup, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CAdvHCFAPinSetup)
	ON_EVENT(CAdvHCFAPinSetup, IDC_PROVIDERS_COMBO, 16 /* SelChosen */, OnSelChosenProvidersCombo, VTS_I4)
	ON_EVENT(CAdvHCFAPinSetup, IDC_LOCATIONS_COMBO, 16 /* SelChosen */, OnSelChosenLocationsCombo, VTS_I4)
	ON_EVENT(CAdvHCFAPinSetup, IDC_PROVIDERS_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedProvidersCombo, VTS_I2)
	ON_EVENT(CAdvHCFAPinSetup, IDC_LOCATIONS_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedLocationsCombo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CAdvHCFAPinSetup::OnSelChosenProvidersCombo(long nRow) 
{
	if(nRow == -1)
		return;

	// (j.jones 2007-04-05 09:15) - PLID 25496 - see if they changed providers,
	// if so, warn and save first

	if(m_ProviderID != -1) {
		//PLID 21711 - check to see if they are still on the same provider
		long nProvID = VarLong(m_ProvidersCombo->GetValue(nRow, 0));
		if (nProvID != m_ProviderID) {
			if (m_bHasChanged) {
				if(IDYES == MessageBox("Any changes made to the previous provider / location combination will be saved.\n"
					"Do you still wish to switch providers?","Practice",MB_ICONQUESTION|MB_YESNO)) {
	
					// (a.walling 2008-05-23 14:31) - PLID 27810 - Cancelable
					if (!Save()) {
						m_ProvidersCombo->SetSelByColumn(0,(long)m_ProviderID);
						return;
					}
				}
				else {
					//don't load and overwrite their changes yet
					m_ProvidersCombo->SetSelByColumn(0,(long)m_ProviderID);
					return;
				}
			}
		}
		else {
			//don't load and overwrite the changes
			return;
		}
	}

	m_ProviderID = VarLong(m_ProvidersCombo->GetValue(nRow,0),-1);

	Load();
}

void CAdvHCFAPinSetup::OnSelChosenLocationsCombo(long nRow) 
{
	if(nRow == -1)
		return;

	// (j.jones 2007-04-05 09:15) - PLID 25496 - see if they changed locations,
	// if so, warn and save first

	if(m_LocationID != -1) {
		//check to see if they are still on the same location
		long nLocID = VarLong(m_LocationsCombo->GetValue(nRow, 0));
		if (nLocID != m_LocationID) {
			if (m_bHasChanged) {
				if(IDYES == MessageBox("Any changes made to the previous provider / location combination will be saved.\n"
					"Do you still wish to switch locations?","Practice",MB_ICONQUESTION|MB_YESNO)) {
	
					// (a.walling 2008-05-23 14:32) - PLID 27810 - Cancelable
					if (!Save()) {
						m_LocationsCombo->SetSelByColumn(0,(long)m_LocationID);
						return;
					}
				}
				else {
					//don't load and overwrite their changes yet
					m_LocationsCombo->SetSelByColumn(0,(long)m_LocationID);
					return;
				}
			}
		}
		else {
			//don't load and overwrite the changes
			return;
		}
	}

	m_LocationID = VarLong(m_LocationsCombo->GetValue(nRow,0),-1);

	Load();
}

void CAdvHCFAPinSetup::Load()
{
	if(m_ProvidersCombo->CurSel == -1 || m_LocationsCombo->CurSel == -1)
		return;

	m_bHasChanged = FALSE;
	m_bHas33bQualChanged = FALSE;
	m_bIsLoading = TRUE;

	try {

		// (j.jones 2008-09-10 11:23) - PLID 30788 - added Box25Check
		// (j.jones 2010-02-01 09:31) - PLID 37137 - added the Box 33 address override
		_RecordsetPtr rs = CreateParamRecordset("SELECT Box33BQual, Box24JQual, PIN, GRP, EIN, Box24J, Box19, Box33Name, "
			"Box24JNPI, Box33aNPI, Box25Check, "
			"Box33_Address1, Box33_Address2, Box33_City, Box33_State, Box33_Zip "
			"FROM AdvHCFAPinT WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",
			VarLong(m_ProvidersCombo->GetValue(m_ProvidersCombo->CurSel,0)),
			VarLong(m_LocationsCombo->GetValue(m_LocationsCombo->CurSel,0)),
			m_HCFAGroup);

		CString Box33BQual= "", Box24JQual = "", PIN = "", GRP = "", EIN = "", Box24J = "", Box19 = "", Box33Name = "",
			Box24JNPI = "", Box33aNPI = "", strBox33Address1 = "", strBox33Address2 = "", strBox33City = "", strBox33State = "", strBox33Zip = "";
		long nBox25Check = 0;

		if(!rs->eof) {
			_variant_t var;

			var = rs->Fields->Item["Box33BQual"]->Value;
			if(var.vt == VT_BSTR)
				Box33BQual = CString(var.bstrVal);

			var = rs->Fields->Item["Box24JQual"]->Value;
			if(var.vt == VT_BSTR)
				Box24JQual = CString(var.bstrVal);

			var = rs->Fields->Item["PIN"]->Value;
			if(var.vt == VT_BSTR)
				PIN = CString(var.bstrVal);

			// (j.jones 2007-02-26 09:16) - PLID 24908 - re-added GRP box
			var = rs->Fields->Item["GRP"]->Value;
			if(var.vt == VT_BSTR)
				GRP = CString(var.bstrVal);

			var = rs->Fields->Item["EIN"]->Value;
			if(var.vt == VT_BSTR)
				EIN = CString(var.bstrVal);

			var = rs->Fields->Item["Box24J"]->Value;
			if(var.vt == VT_BSTR)
				Box24J = CString(var.bstrVal);

			var = rs->Fields->Item["Box19"]->Value;
			if(var.vt == VT_BSTR)
				Box19 = CString(var.bstrVal);

			// (j.jones 2007-05-07 13:58) - PLID 25922 - added Box 33 Name override
			var = rs->Fields->Item["Box33Name"]->Value;
			if(var.vt == VT_BSTR)
				Box33Name = CString(var.bstrVal);

			// (j.jones 2007-08-08 10:04) - PLID 25395 - added the NPI fields
			// (a.walling 2008-05-19 17:35) - PLID 27810 - Saved the loaded values so as not to warn them of invalid NPIs if nothing changed.
			var = rs->Fields->Item["Box24JNPI"]->Value;
			if(var.vt == VT_BSTR)
				Box24JNPI = m_strOriginal24JNPI = CString(var.bstrVal);

			var = rs->Fields->Item["Box33aNPI"]->Value;
			if(var.vt == VT_BSTR)
				Box33aNPI = m_strOriginal33aNPI = CString(var.bstrVal);

			// (j.jones 2008-09-10 11:24) - PLID 30788 - added Box25Check
			nBox25Check = AdoFldLong(rs, "Box25Check", 0);

			// (j.jones 2010-02-01 09:31) - PLID 37137 - added the Box 33 address override
			strBox33Address1 = AdoFldString(rs, "Box33_Address1", "");
			strBox33Address2 = AdoFldString(rs, "Box33_Address2", "");
			strBox33City = AdoFldString(rs, "Box33_City", "");
			strBox33State = AdoFldString(rs, "Box33_State", "");
			strBox33Zip = AdoFldString(rs, "Box33_Zip", "");
		}
		rs->Close();

		SetDlgItemText(IDC_BOX33_QUAL, Box33BQual);
		SetDlgItemText(IDC_BOX24J_QUAL, Box24JQual);
		SetDlgItemText(IDC_PIN,PIN);
		SetDlgItemText(IDC_ADV_EIN,EIN);
		SetDlgItemText(IDC_ADV_BOX24J,Box24J);
		SetDlgItemText(IDC_ADV_BOX19,Box19);
		SetDlgItemText(IDC_ADV_GRP,GRP);
		SetDlgItemText(IDC_ADV_BOX33_NAME,Box33Name);
		SetDlgItemText(IDC_ADV_BOX24J_NPI,Box24JNPI);
		SetDlgItemText(IDC_ADV_BOX33A_NPI,Box33aNPI);
		// (j.jones 2010-02-01 09:31) - PLID 37137 - added the Box 33 address override
		SetDlgItemText(IDC_ADV_BOX33_ADDRESS1,strBox33Address1);
		SetDlgItemText(IDC_ADV_BOX33_ADDRESS2,strBox33Address2);
		SetDlgItemText(IDC_ADV_BOX33_CITY,strBox33City);
		SetDlgItemText(IDC_ADV_BOX33_STATE,strBox33State);
		SetDlgItemText(IDC_ADV_BOX33_ZIP,strBox33Zip);

		// (j.jones 2008-09-10 11:24) - PLID 30788 - added Box25Check
		//0 - none, 1 - SSN, 2 - EIN
		if(nBox25Check == 1) {
			m_radioSSN.SetCheck(TRUE);
			m_radioEIN.SetCheck(FALSE);
			m_radioNoOverride.SetCheck(FALSE);
		}
		else if(nBox25Check == 2) {
			m_radioSSN.SetCheck(FALSE);
			m_radioEIN.SetCheck(TRUE);
			m_radioNoOverride.SetCheck(FALSE);
		}
		else {
			m_radioSSN.SetCheck(FALSE);
			m_radioEIN.SetCheck(FALSE);
			m_radioNoOverride.SetCheck(TRUE);
		}

		m_bIsLoading = FALSE;		

	}NxCatchAll("Error loading advanced HCFA PIN settings.");
}

// (a.walling 2008-05-23 14:30) - PLID 27810 - Cancelable
BOOL CAdvHCFAPinSetup::Save()
{
	if(m_ProvidersCombo->CurSel == -1 || m_LocationsCombo->CurSel == -1)
		return FALSE;

	try {

		CString Box33BQual = "", Box24JQual = "", PIN = "", GRP = "", EIN = "", Box24J = "", Box19 = "", Box33Name = "",
			Box24JNPI = "", Box33aNPI = "", strBox33Address1 = "", strBox33Address2 = "", strBox33City = "", strBox33State = "", strBox33Zip = "";

		long nBox25Check = 0;

		GetDlgItemText(IDC_BOX33_QUAL, Box33BQual);
		GetDlgItemText(IDC_BOX24J_QUAL, Box24JQual);
		GetDlgItemText(IDC_PIN,PIN);
		// (j.jones 2007-02-26 09:16) - PLID 24908 - re-added GRP box
		GetDlgItemText(IDC_ADV_GRP,GRP);
		GetDlgItemText(IDC_ADV_EIN,EIN);
		GetDlgItemText(IDC_ADV_BOX24J,Box24J);
		GetDlgItemText(IDC_ADV_BOX19,Box19);
		// (j.jones 2007-05-07 14:02) - PLID 25922 - added Box 33 Name override
		GetDlgItemText(IDC_ADV_BOX33_NAME,Box33Name);
		// (j.jones 2007-08-08 10:04) - PLID 25395 - added the NPI fields
		GetDlgItemText(IDC_ADV_BOX24J_NPI,Box24JNPI);
		GetDlgItemText(IDC_ADV_BOX33A_NPI,Box33aNPI);
		// (j.jones 2010-02-01 09:31) - PLID 37137 - added the Box 33 address override
		GetDlgItemText(IDC_ADV_BOX33_ADDRESS1,strBox33Address1);
		GetDlgItemText(IDC_ADV_BOX33_ADDRESS2,strBox33Address2);
		GetDlgItemText(IDC_ADV_BOX33_CITY,strBox33City);
		GetDlgItemText(IDC_ADV_BOX33_STATE,strBox33State);
		GetDlgItemText(IDC_ADV_BOX33_ZIP,strBox33Zip);

		Box33BQual.TrimLeft();
		Box33BQual.TrimRight();

		// (j.jones 2010-04-16 08:43) - PLID 38149 - if they changed the Box 33b qualifier,
		// warn if it is XX (doesn't matter if it already was XX, so long as they touched the box)
		if(m_bHas33bQualChanged && Box33BQual.CompareNoCase("XX") == 0) {
			if(IDNO == MessageBox("The Box33b qualifier is XX, which is not a valid qualifier to use.\n"
				"This configuration may cause your claims to be rejected.\n\n"
				"Are you sure you want to send XX as a Box 33b qualifier?", "Practice", MB_ICONQUESTION|MB_YESNO)) {
				//make it clear that nothing was saved
				AfxMessageBox("The save has been cancelled, please correct the Box33b qualifier before continuing.");
				return FALSE;
			}
		}

		if(Box33BQual == "")
			Box33BQual = "NULL";
		else
			Box33BQual = "'" + _Q(Box33BQual) + "'";

		Box24JQual.TrimLeft();
		Box24JQual.TrimRight();

		if(Box24JQual == "")
			Box24JQual = "NULL";
		else
			Box24JQual = "'" + _Q(Box24JQual) + "'";

		PIN.TrimLeft();
		PIN.TrimRight();

		if(PIN == "")
			PIN = "NULL";
		else
			PIN = "'" + _Q(PIN) + "'";

		GRP.TrimLeft();
		GRP.TrimRight();

		if(GRP == "")
			GRP = "NULL";
		else
			GRP = "'" + _Q(GRP) + "'";

		EIN.TrimLeft();
		EIN.TrimRight();

		if(EIN == "")
			EIN = "NULL";
		else
			EIN = "'" + _Q(EIN) + "'";

		Box24J.TrimLeft();
		Box24J.TrimRight();

		if(Box24J == "")
			Box24J = "NULL";
		else
			Box24J = "'" + _Q(Box24J) + "'";

		Box19.TrimLeft();
		Box19.TrimRight();

		if(Box19 == "")
			Box19 = "NULL";
		else
			Box19 = "'" + _Q(Box19) + "'";

		Box33Name.TrimLeft();
		Box33Name.TrimRight();

		if(Box33Name == "")
			Box33Name = "NULL";
		else
			Box33Name = "'" + _Q(Box33Name) + "'";

		Box24JNPI.TrimLeft();
		Box24JNPI.TrimRight();

		if (m_strOriginal24JNPI != Box24JNPI) {
			// (a.walling 2008-05-19 17:21) - PLID 27810 - Check validity of NPIs
			if (!CheckNPI(Box24JNPI, this, TRUE, "entered for Box 24J ")) {
				return FALSE;
			}
		}

		if(Box24JNPI == "")
			Box24JNPI = "NULL";
		else
			Box24JNPI = "'" + _Q(Box24JNPI) + "'";

		Box33aNPI.TrimLeft();
		Box33aNPI.TrimRight();

		if (m_strOriginal33aNPI != Box33aNPI) {
			// (a.walling 2008-05-19 17:21) - PLID 27810 - Check validity of NPIs
			if (!CheckNPI(Box33aNPI, this, TRUE, "entered for Box 33a ")) {
				return FALSE;
			}
		}

		if(Box33aNPI == "")
			Box33aNPI = "NULL";
		else
			Box33aNPI = "'" + _Q(Box33aNPI) + "'";

		// (j.jones 2008-09-10 11:24) - PLID 30788 - added Box25Check
		//0 - none, 1 - SSN, 2 - EIN
		if(m_radioSSN.GetCheck()) {
			nBox25Check = 1;
		}
		else if(m_radioEIN.GetCheck()) {
			nBox25Check = 2;
		}
		else {
			nBox25Check = 0;
		}

		// (j.jones 2010-02-01 09:31) - PLID 37137 - added the Box 33 address override
		strBox33Address1.TrimLeft();
		strBox33Address1.TrimRight();
		if(strBox33Address1.IsEmpty()) {
			strBox33Address1 = "NULL";
		}
		else {
			strBox33Address1 = "'" + _Q(strBox33Address1) + "'";
		}
		
		strBox33Address2.TrimLeft();
		strBox33Address2.TrimRight();
		if(strBox33Address2.IsEmpty()) {
			strBox33Address2 = "NULL";
		}
		else {
			strBox33Address2 = "'" + _Q(strBox33Address2) + "'";
		}

		strBox33City.TrimLeft();
		strBox33City.TrimRight();
		if(strBox33City.IsEmpty()) {
			strBox33City = "NULL";
		}
		else {
			strBox33City = "'" + _Q(strBox33City) + "'";
		}

		strBox33State.TrimLeft();
		strBox33State.TrimRight();
		if(strBox33State.IsEmpty()) {
			strBox33State = "NULL";
		}
		else {
			strBox33State = "'" + _Q(strBox33State) + "'";
		}

		strBox33Zip.TrimLeft();
		strBox33Zip.TrimRight();
		if(strBox33Zip.IsEmpty()) {
			strBox33Zip = "NULL";
		}
		else {
			strBox33Zip = "'" + _Q(strBox33Zip) + "'";
		}

		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 ProviderID FROM AdvHCFAPinT "
			"WHERE ProviderID = {INT} AND LocationID = {INT} AND SetupGroupID = {INT}",
			m_ProviderID, m_LocationID,m_HCFAGroup);

		if(rs->eof) {
			ExecuteSql("INSERT INTO AdvHCFAPinT (ProviderID, LocationID, SetupGroupID) VALUES (%li,%li,%li)",m_ProviderID,m_LocationID,m_HCFAGroup);
		}
		rs->Close();

		// (j.jones 2007-02-26 09:16) - PLID 24908 - re-added GRP box
		// (j.jones 2007-05-07 14:16) - PLID 25922 - added Box 33 Name override
		// (j.jones 2007-08-08 10:04) - PLID 25395 - added the NPI fields
		// (j.jones 2008-09-10 11:24) - PLID 30788 - added Box25Check
		// (j.jones 2010-02-01 09:31) - PLID 37137 - added the Box 33 address override
		ExecuteSql("UPDATE AdvHCFAPinT SET Box33BQual = %s, Box24JQual = %s, PIN = %s, GRP = %s, EIN = %s, Box24J = %s, Box19 = %s, Box33Name = %s, "
			"Box24JNPI = %s, Box33aNPI = %s, Box25Check = %li, "
			"Box33_Address1 = %s, Box33_Address2 = %s, Box33_City = %s, Box33_State = %s, Box33_Zip = %s "
			"WHERE ProviderID = %li AND LocationID = %li AND SetupGroupID = %li",
			Box33BQual,Box24JQual,PIN,GRP,EIN,Box24J,Box19,Box33Name, Box24JNPI, Box33aNPI, nBox25Check,
			strBox33Address1, strBox33Address2, strBox33City, strBox33State, strBox33Zip,
			m_ProviderID,m_LocationID,m_HCFAGroup);

	}NxCatchAll("Error saving advanced HCFA PIN settings.");
	
	return TRUE;
}

void CAdvHCFAPinSetup::OnRequeryFinishedProvidersCombo(short nFlags) 
{
	m_ProvidersCombo->CurSel = 0;

	if(m_ProvidersCombo->CurSel != -1)
		m_ProviderID = VarLong(m_ProvidersCombo->GetValue(m_ProvidersCombo->CurSel,0),-1);	

	m_bProvidersLoaded = TRUE;

	if(m_bLocationsLoaded)
		Load();
}

void CAdvHCFAPinSetup::OnRequeryFinishedLocationsCombo(short nFlags) 
{
	m_LocationsCombo->CurSel = 0;

	if(m_LocationsCombo->CurSel != -1)
		m_LocationID = VarLong(m_LocationsCombo->GetValue(m_LocationsCombo->CurSel,0),-1);

	m_bLocationsLoaded = TRUE;	

	if(m_bProvidersLoaded)
		Load();
}

BOOL CAdvHCFAPinSetup::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch (HIWORD(wParam))
	{	
		// (j.jones 2008-09-10 11:31) - PLID 30788 - added support for radio buttons
		case BN_CLICKED:
		{			
			long nID = LOWORD(wParam);

			//this sets m_bHasChanged even if they click a radio button
			//that didn't actually change anything, but that's ok

			if (!m_bIsLoading
				&& nID != IDOK
				&& nID != IDCANCEL) {

				m_bHasChanged = TRUE;
			}
			break;
		}
		case EN_CHANGE:
			if(!m_bIsLoading) {
				m_bHasChanged = TRUE;

				// (j.jones 2010-04-16 08:43) - PLID 38149 - see if they changed the Box 33b qualifier
				long nID = LOWORD(wParam);
				if(nID == IDC_BOX33_QUAL) {
					m_bHas33bQualChanged = TRUE;
				}
			}			
		break;			
	}
	
	return CDialog::OnCommand(wParam, lParam);
}

// (b.eyers 2015-04-09) - PLID 59169 - Autofill in city and state if zip is entered, tab order was also updated
void CAdvHCFAPinSetup::OnKillfocusZipBox()
{
	try {
		if (!m_bLookupByCity) {
			CString city,
				state,
				tempCity,
				tempState,
				value;
			GetDlgItemText(IDC_ADV_BOX33_ZIP, value);
			GetDlgItemText(IDC_ADV_BOX33_CITY, tempCity);
			GetDlgItemText(IDC_ADV_BOX33_STATE, tempState);
			tempCity.TrimRight();
			tempState.TrimRight();
			if (!tempCity.IsEmpty() || !tempState.IsEmpty()) {
				MAINTAIN_FOCUS(); 
				if (AfxMessageBox("You have changed the postal code but the city or state already have data in them.  Would you like to overwrite "
					"this data with that of the new postal code?", MB_YESNO) == IDYES)
				{
					tempCity.Empty();
					tempState.Empty();
				}
			}
			if (tempCity == "" || tempState == "") {
				GetZipInfo(value, &city, &state);

				if (city == "" && state == ""){
					CString str;
					str = value.Left(5);
					GetZipInfo(str, &city, &state);
				}
				if (tempCity == "")
					SetDlgItemText(IDC_ADV_BOX33_CITY, city);
				if (tempState == "")
					SetDlgItemText(IDC_ADV_BOX33_STATE, state);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.eyers 2015-04-09) - PLID 59169 - Autofill in zip and state if city is entered
void CAdvHCFAPinSetup::OnKillfocusCityBox()
{
	try {
		if (m_bLookupByCity) {
			CString zip,
				state,
				tempZip,
				tempState,
				value;
			GetDlgItemText(IDC_ADV_BOX33_CITY, value);
			GetDlgItemText(IDC_ADV_BOX33_ZIP, tempZip);
			GetDlgItemText(IDC_ADV_BOX33_STATE, tempState);
			tempZip.TrimRight();
			tempState.TrimRight();
			if (!tempZip.IsEmpty() || !tempState.IsEmpty()) {
				MAINTAIN_FOCUS(); 
				if (AfxMessageBox("You have changed the city but the postal code or state already have data in them.  Would you like to overwrite "
					"this data with that of the new city?", MB_YESNO) == IDYES)
				{
					tempZip.Empty();
					tempState.Empty();
				}
			}
			if (tempZip == "" || tempState == "") {
				GetCityInfo(value, &zip, &state);
				if (tempZip == "")
					SetDlgItemText(IDC_ADV_BOX33_ZIP, zip);
				if (tempState == "")
					SetDlgItemText(IDC_ADV_BOX33_STATE, state);
			}
		}
	}NxCatchAll(__FUNCTION__);
}