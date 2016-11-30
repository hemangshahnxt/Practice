// Adv2010ABPayToOverrideDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Adv2010ABPayToOverrideDlg.h"

// (j.jones 2010-11-01 09:40) - PLID 40919 - created

// CAdv2010ABPayToOverrideDlg dialog

using namespace ADODB;

IMPLEMENT_DYNAMIC(CAdv2010ABPayToOverrideDlg, CNxDialog)

CAdv2010ABPayToOverrideDlg::CAdv2010ABPayToOverrideDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CAdv2010ABPayToOverrideDlg::IDD, pParent)
{
	m_nProviderID = -1;
	m_nLocationID = -1;
	m_bIsUB04 = FALSE;
	m_nSetupGroupID = -1;
	m_b2010AA = FALSE;
}

CAdv2010ABPayToOverrideDlg::~CAdv2010ABPayToOverrideDlg()
{
}

void CAdv2010ABPayToOverrideDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDOK, m_btnOk);
	DDX_Control(pDX, IDCANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_2010AB_PROVIDER_LABEL, m_nxstaticProviderLabel);
	DDX_Control(pDX, IDC_2010AB_LOCATION_LABEL, m_nxstaticLocationLabel);
	DDX_Control(pDX, IDC_EDIT_2010AB_ADDRESS1, m_nxeditAddress1);
	DDX_Control(pDX, IDC_EDIT_2010AB_ADDRESS2, m_nxeditAddress2);
	DDX_Control(pDX, IDC_EDIT_2010AB_CITY, m_nxeditCity);
	DDX_Control(pDX, IDC_EDIT_2010AB_STATE, m_nxeditState);
	DDX_Control(pDX, IDC_EDIT_2010AB_ZIP, m_nxeditZip);
	DDX_Control(pDX, IDC_2010AB_INFO_LABEL, m_nxstaticInfoLabel);
}

BEGIN_MESSAGE_MAP(CAdv2010ABPayToOverrideDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, OnOK)
	ON_EN_KILLFOCUS(IDC_EDIT_2010AB_ZIP, OnKillfocusZipBox) // (b.eyers 2015-04-09) - PLID 59169
	ON_EN_KILLFOCUS(IDC_EDIT_2010AB_CITY, OnKillfocusCityBox) // (b.eyers 2015-04-09) - PLID 59169
END_MESSAGE_MAP()

// CAdv2010ABPayToOverrideDlg message handlers

// (j.jones 2011-11-16 14:53) - PLID 46489 - added parameter for when this is overriding 2010AA and not 2010AB
int CAdv2010ABPayToOverrideDlg::DoModal(long nProviderID, long nLocationID, BOOL bIsUB04, long nSetupGroupID, BOOL b2010AA)
{
	try {

		//should be impossible to call this with -1 IDs, means the calling code fails
		if(nProviderID == -1 || nLocationID == -1 || nSetupGroupID == -1) {
			ThrowNxException("Cannot load the address overrides, invalid provider (%li), location (%li), group ID (%li)!", nProviderID, nLocationID, nSetupGroupID);
		}

		m_nProviderID = nProviderID;
		m_nLocationID = nLocationID;
		m_bIsUB04 = bIsUB04;
		m_nSetupGroupID = nSetupGroupID;
		m_b2010AA = b2010AA;

		return CNxDialog::DoModal();

	}NxCatchAll(__FUNCTION__);

	return IDCANCEL;
}

BOOL CAdv2010ABPayToOverrideDlg::OnInitDialog()
{
	try {

		CNxDialog::OnInitDialog();

		m_btnOk.AutoSet(NXB_OK);
		m_btnCancel.AutoSet(NXB_CANCEL);

		//technically ANSI does not support the lengths
		//that we do in PersonT, but we are replicating
		//the PersonT lengths here
		m_nxeditAddress1.SetLimitText(75);
		m_nxeditAddress2.SetLimitText(75);
		m_nxeditCity.SetLimitText(50);
		m_nxeditState.SetLimitText(20);
		m_nxeditZip.SetLimitText(20);

		// (j.jones 2011-11-16 14:53) - PLID 46489 - this dialog can now optionally override 2010AA and not 2010AB
		if(m_b2010AA) {
			SetWindowText("Override 2010AA Billing Provider Address");
			m_nxstaticInfoLabel.SetWindowText("Loop 2010AA is a Billing Provider Address, and must be a street address. It cannot be a P.O. Box.");
		}

		//load our data
		_RecordsetPtr rs;
		if(m_bIsUB04) {
			//UB

			//left join UB92EbillingSetupT, it might not exist yet
			// (j.jones 2011-11-10 15:40) - PLID 46405 - filter on UB92EbillingSetupT *prior* to the left join
			// (j.jones 2011-11-16 14:53) - PLID 46489 - this dialog can now optionally override 2010AA and not 2010AB
			if(m_b2010AA) {
				rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle + CASE WHEN Title <> '' THEN ', ' + Title ELSE '' END AS ProviderName, "
					"LocationsT.Name AS LocationName, UB92EbillingSetupQ.SetupGroupID, "
					"PayTo2010AA_Address1 AS Address1, PayTo2010AA_Address2 AS Address2, "
					"PayTo2010AA_City AS City, PayTo2010AA_State AS State, PayTo2010AA_Zip AS Zip "
					"FROM ProvidersT "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
					"CROSS JOIN LocationsT "
					"LEFT JOIN (SELECT * FROM UB92EbillingSetupT WHERE SetupGroupID = {INT}) AS UB92EbillingSetupQ ON ProvidersT.PersonID = UB92EbillingSetupQ.ProviderID "
					"	AND LocationsT.ID = UB92EbillingSetupQ.LocationID "
					"WHERE ProvidersT.PersonID = {INT} AND LocationsT.ID = {INT}", m_nSetupGroupID, m_nProviderID, m_nLocationID);
			}
			else {
				rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle + CASE WHEN Title <> '' THEN ', ' + Title ELSE '' END AS ProviderName, "
					"LocationsT.Name AS LocationName, UB92EbillingSetupQ.SetupGroupID, "
					"PayTo2010AB_Address1 AS Address1, PayTo2010AB_Address2 AS Address2, "
					"PayTo2010AB_City AS City, PayTo2010AB_State AS State, PayTo2010AB_Zip AS Zip "
					"FROM ProvidersT "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
					"CROSS JOIN LocationsT "
					"LEFT JOIN (SELECT * FROM UB92EbillingSetupT WHERE SetupGroupID = {INT}) AS UB92EbillingSetupQ ON ProvidersT.PersonID = UB92EbillingSetupQ.ProviderID "
					"	AND LocationsT.ID = UB92EbillingSetupQ.LocationID "
					"WHERE ProvidersT.PersonID = {INT} AND LocationsT.ID = {INT}", m_nSetupGroupID, m_nProviderID, m_nLocationID);
			}
		}
		else {
			//HCFA

			//left join EbillingSetupT, it might not exist yet
			// (j.jones 2011-11-10 15:40) - PLID 46405 - filter on EbillingSetupT *prior* to the left join
			// (j.jones 2011-11-16 14:53) - PLID 46489 - this dialog can now optionally override 2010AA and not 2010AB
			if(m_b2010AA) {
				rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle + CASE WHEN Title <> '' THEN ', ' + Title ELSE '' END AS ProviderName, "
					"LocationsT.Name AS LocationName, EbillingSetupQ.SetupGroupID, "
					"PayTo2010AA_Address1 AS Address1, PayTo2010AA_Address2 AS Address2, "
					"PayTo2010AA_City AS City, PayTo2010AA_State AS State, PayTo2010AA_Zip AS Zip "
					"FROM ProvidersT "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
					"CROSS JOIN LocationsT "
					"LEFT JOIN (SELECT * FROM EbillingSetupT WHERE SetupGroupID = {INT}) AS EbillingSetupQ ON ProvidersT.PersonID = EbillingSetupQ.ProviderID "
					"	AND LocationsT.ID = EbillingSetupQ.LocationID "
					"WHERE ProvidersT.PersonID = {INT} AND LocationsT.ID = {INT}", m_nSetupGroupID, m_nProviderID, m_nLocationID);
			}
			else {
				rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle + CASE WHEN Title <> '' THEN ', ' + Title ELSE '' END AS ProviderName, "
					"LocationsT.Name AS LocationName, EbillingSetupQ.SetupGroupID, "
					"PayTo2010AB_Address1 AS Address1, PayTo2010AB_Address2 AS Address2, "
					"PayTo2010AB_City AS City, PayTo2010AB_State AS State, PayTo2010AB_Zip AS Zip "
					"FROM ProvidersT "
					"INNER JOIN PersonT ON ProvidersT.PersonID = PersonT.ID "
					"CROSS JOIN LocationsT "
					"LEFT JOIN (SELECT * FROM EbillingSetupT WHERE SetupGroupID = {INT}) AS EbillingSetupQ ON ProvidersT.PersonID = EbillingSetupQ.ProviderID "
					"	AND LocationsT.ID = EbillingSetupQ.LocationID "
					"WHERE ProvidersT.PersonID = {INT} AND LocationsT.ID = {INT}", m_nSetupGroupID, m_nProviderID, m_nLocationID);
			}
		}

		if(rs->eof) {
			//should not be possible, we should have caught this in DoModal()
			ThrowNxException("Cannot load the 2010AB overrides, invalid provider (%li), location (%li), group ID (%li)!", m_nProviderID, m_nLocationID, m_nSetupGroupID);
		}

		//check to see if we have a valid setup entry
		long nTestID = AdoFldLong(rs, "SetupGroupID", -1);
		if(nTestID == -1) {
			//there is no EbillingSetupT record, create it
			if(m_bIsUB04) {
				ExecuteParamSql("INSERT INTO UB92EbillingSetupT (SetupGroupID, ProviderID, LocationID) "
					"VALUES ({INT}, {INT}, {INT})", m_nSetupGroupID, m_nProviderID, m_nLocationID);
			}
			else {
				ExecuteParamSql("INSERT INTO EbillingSetupT (SetupGroupID, ProviderID, LocationID) "
					"VALUES ({INT}, {INT}, {INT})", m_nSetupGroupID, m_nProviderID, m_nLocationID);
			}
		}

		CString strProviderName = AdoFldString(rs, "ProviderName", "");
		CString strLocationName = AdoFldString(rs, "LocationName", "");
		CString strAddress1 = AdoFldString(rs, "Address1", "");
		CString strAddress2 = AdoFldString(rs, "Address2", "");
		CString strCity = AdoFldString(rs, "City", "");
		CString strState = AdoFldString(rs, "State", "");
		CString strZip = AdoFldString(rs, "Zip", "");

		m_nxstaticProviderLabel.SetWindowText(strProviderName);
		m_nxstaticLocationLabel.SetWindowText(strLocationName);
		m_nxeditAddress1.SetWindowText(strAddress1);
		m_nxeditAddress2.SetWindowText(strAddress2);
		m_nxeditCity.SetWindowText(strCity);
		m_nxeditState.SetWindowText(strState);
		m_nxeditZip.SetWindowText(strZip);

		rs->Close();

		// (b.eyers 2015-04-09) - PLID 59169
		g_propManager.CachePropertiesInBulk("AdvHCFAPinSetup", propNumber,
			"(Username = '<None>' OR Username = '%s') AND Name IN ( "
			"	'LookupZipStateByCity' "
			")", _Q(GetCurrentUserName()));

		m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");

		if (m_bLookupByCity) {
			ChangeZOrder(IDC_EDIT_2010AB_ZIP, IDC_EDIT_2010AB_STATE);
		}

	}NxCatchAll(__FUNCTION__);

	return TRUE;
}

void CAdv2010ABPayToOverrideDlg::OnOK()
{
	try {

		//save the changes, we have already ensured that an EbillingSetupT
		//entry exists, so just update it

		CString strAddress1;
		CString strAddress2;
		CString strCity;
		CString strState;
		CString strZip;

		m_nxeditAddress1.GetWindowText(strAddress1);
		m_nxeditAddress2.GetWindowText(strAddress2);
		m_nxeditCity.GetWindowText(strCity);
		m_nxeditState.GetWindowText(strState);
		m_nxeditZip.GetWindowText(strZip);

		strAddress1.TrimLeft(); strAddress1.TrimRight();
		strAddress2.TrimLeft(); strAddress2.TrimRight();
		strCity.TrimLeft(); strCity.TrimRight();
		strState.TrimLeft(); strState.TrimRight();
		strZip.TrimLeft(); strZip.TrimRight();

		_variant_t varAddress1 = g_cvarNull;
		_variant_t varAddress2 = g_cvarNull;
		_variant_t varCity = g_cvarNull;
		_variant_t varState = g_cvarNull;
		_variant_t varZip = g_cvarNull;

		if(!strAddress1.IsEmpty()) {
			varAddress1 = (LPCTSTR)strAddress1;
		}
		if(!strAddress2.IsEmpty()) {
			varAddress2 = (LPCTSTR)strAddress2;
		}
		if(!strCity.IsEmpty()) {
			varCity = (LPCTSTR)strCity;
		}
		if(!strState.IsEmpty()) {
			varState = (LPCTSTR)strState;
		}
		if(!strZip.IsEmpty()) {
			varZip = (LPCTSTR)strZip;
		}

		//finally save
		if(m_bIsUB04) {
			// (j.jones 2011-11-16 14:53) - PLID 46489 - this dialog can now optionally override 2010AA and not 2010AB
			if(m_b2010AA) {
				ExecuteParamSql("UPDATE UB92EbillingSetupT SET "
					"PayTo2010AA_Address1 = {VT_BSTR}, PayTo2010AA_Address2 = {VT_BSTR}, "
					"PayTo2010AA_City = {VT_BSTR}, PayTo2010AA_State = {VT_BSTR}, PayTo2010AA_Zip = {VT_BSTR} "
					"WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
					varAddress1, varAddress2,
					varCity, varState, varZip,
					m_nSetupGroupID, m_nProviderID, m_nLocationID);
			}
			else {
				ExecuteParamSql("UPDATE UB92EbillingSetupT SET "
					"PayTo2010AB_Address1 = {VT_BSTR}, PayTo2010AB_Address2 = {VT_BSTR}, "
					"PayTo2010AB_City = {VT_BSTR}, PayTo2010AB_State = {VT_BSTR}, PayTo2010AB_Zip = {VT_BSTR} "
					"WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
					varAddress1, varAddress2,
					varCity, varState, varZip,
					m_nSetupGroupID, m_nProviderID, m_nLocationID);
			}
		}
		else {
			// (j.jones 2011-11-16 14:53) - PLID 46489 - this dialog can now optionally override 2010AA and not 2010AB
			if(m_b2010AA) {
				ExecuteParamSql("UPDATE EbillingSetupT SET "
					"PayTo2010AA_Address1 = {VT_BSTR}, PayTo2010AA_Address2 = {VT_BSTR}, "
					"PayTo2010AA_City = {VT_BSTR}, PayTo2010AA_State = {VT_BSTR}, PayTo2010AA_Zip = {VT_BSTR} "
					"WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
					varAddress1, varAddress2,
					varCity, varState, varZip,
					m_nSetupGroupID, m_nProviderID, m_nLocationID);
			}
			else {
				ExecuteParamSql("UPDATE EbillingSetupT SET "
					"PayTo2010AB_Address1 = {VT_BSTR}, PayTo2010AB_Address2 = {VT_BSTR}, "
					"PayTo2010AB_City = {VT_BSTR}, PayTo2010AB_State = {VT_BSTR}, PayTo2010AB_Zip = {VT_BSTR} "
					"WHERE SetupGroupID = {INT} AND ProviderID = {INT} AND LocationID = {INT}",
					varAddress1, varAddress2,
					varCity, varState, varZip,
					m_nSetupGroupID, m_nProviderID, m_nLocationID);
			}
		}

		CNxDialog::OnOK();

	}NxCatchAll(__FUNCTION__);
}

// (b.eyers 2015-04-09) - PLID 59169 - Autofill in city and state if zip is entered, tab order was also updated
void CAdv2010ABPayToOverrideDlg::OnKillfocusZipBox()
{
	try {
		if (!m_bLookupByCity) {
			CString city,
				state,
				tempCity,
				tempState,
				value;
			GetDlgItemText(IDC_EDIT_2010AB_ZIP, value);
			GetDlgItemText(IDC_EDIT_2010AB_CITY, tempCity);
			GetDlgItemText(IDC_EDIT_2010AB_STATE, tempState);
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
					SetDlgItemText(IDC_EDIT_2010AB_CITY, city);
				if (tempState == "")
					SetDlgItemText(IDC_EDIT_2010AB_STATE, state);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (b.eyers 2015-04-09) - PLID 59169 - Autofill in zip and state if city is entered
void CAdv2010ABPayToOverrideDlg::OnKillfocusCityBox()
{
	try {
		if (m_bLookupByCity) {
			CString zip,
				state,
				tempZip,
				tempState,
				value;
			GetDlgItemText(IDC_EDIT_2010AB_CITY, value);
			GetDlgItemText(IDC_EDIT_2010AB_ZIP, tempZip);
			GetDlgItemText(IDC_EDIT_2010AB_STATE, tempState);
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
					SetDlgItemText(IDC_EDIT_2010AB_ZIP, zip);
				if (tempState == "")
					SetDlgItemText(IDC_EDIT_2010AB_STATE, state);
			}
		}
	}NxCatchAll(__FUNCTION__);
}