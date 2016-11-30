// PracticeInfo.cpp : implementation file
//

#include "stdafx.h"
#include "AdministratorRc.h"
#include "PracticeInfo.h"
#include "NxStandard.h"
#include "PracProps.h"
#include "GlobalDataUtils.h"
#include "PlaceCodeDlg.h"
#include "AuditTrail.h"
#include "InternationalUtils.h"
#include "DateTimeUtils.h"
#include "SelectLocationDlg.h"
#include "LocationBiographyDlg.h"
#include "LocationSelectImageDlg.h"
#include "PharmacyIDDlg.h"
#include "EditPharmacyStaffDlg.h"
#include "OHIPUtils.h"
#include "PharmacyDirectorySearchDlg.h"
#include "HL7ParseUtils.h"
#include "AlbertaHLINKUtils.h"
#include "HL7Utils.h"
#include "GlobalUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2007-11-06 09:23) - PLID 28000 - VS2008 - No 'using namespace' within header files
using namespace ADODB;
using namespace NXDATALISTLib;

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.


/////////////////////////////////////////////////////////////////////////////
// CPracticeInfo dialog
// (s.tullis 2016-03-07 11:56) - PLID 68444 
enum ClaimFormList {
	cflID = 0,
	cflName,
};

CPracticeInfo::CPracticeInfo(CWnd* pParent)
	: CNxDialog(CPracticeInfo::IDD, pParent),
	m_ProviderChecker(NetUtils::Providers),
	m_PatCoordChecker(NetUtils::Coordinators)	
{
	//{{AFX_DATA_INIT(CPracticeInfo)
	//}}AFX_DATA_INIT

	// (z.manning, 10/25/2007) - PLID 27868 - We now have a help section for the biography/logo/image
	// stuff for locations, and since we have nothing else that applies here, may as well use that for
	// the default help.
	m_strManualLocation = "NexTech_Practice_Manual.chm";
	m_strManualBookmark = "The_Administrator_Module/Locations/Location_Biography_Logo.htm";
}


void CPracticeInfo::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPracticeInfo)
	DDX_Control(pDX, IDC_EDIT_PLACE_OF_SERVICE, m_btnEditPlaceOfService);
	DDX_Control(pDX, IDC_BUTTON_LOCATION_LOGO, m_btnLogo);
	DDX_Control(pDX, IDC_BUTTON_LOCATION_IMAGE, m_btnImage);
	DDX_Control(pDX, IDC_BUTTON_LOCATION_BIOGRAPHY, m_btnBiography);
	DDX_Control(pDX, IDC_CHECK_DEFAULT_FOR_NEW_PATIENTS, m_checkDefaultForNewPatients);
	DDX_Control(pDX, IDC_LOCATION_ACTIVE, m_buActive);
	DDX_Control(pDX, IDC_PRACTICE_LEFT, m_btnPracticeLeft);
	DDX_Control(pDX, IDC_PRACTICE_RIGHT, m_btnPracticeRight);
	DDX_Control(pDX, IDC_DELETELOCATION, m_btnDelete);
	DDX_Control(pDX, IDC_ADDLOCATION, m_btnAdd);
	DDX_Control(pDX, IDC_REMOVE_ALL, m_remAllBtn);
	DDX_Control(pDX, IDC_REMOVE, m_remBtn);
	DDX_Control(pDX, IDC_ADD_ALL, m_addAllBtn);
	DDX_Control(pDX, IDC_ADD, m_addBtn);
	DDX_Control(pDX, IDC_MANAGED, m_managed);
	DDX_Control(pDX, IDC_NAME, m_nxeditName);
	DDX_Control(pDX, IDC_PRACADDRESS1, m_nxeditPracaddress1);
	DDX_Control(pDX, IDC_PRACADDRESS2, m_nxeditPracaddress2);
	DDX_Control(pDX, IDC_PRACZIP, m_nxeditPraczip);
	DDX_Control(pDX, IDC_PRACCITY, m_nxeditPraccity);
	DDX_Control(pDX, IDC_PRACSTATE, m_nxeditPracstate);
	DDX_Control(pDX, IDC_WEBSITE, m_nxeditWebsite);
	DDX_Control(pDX, IDC_PRACEIN, m_nxeditPracein);
	DDX_Control(pDX, IDC_PRAC_NPI, m_nxeditPracNpi);
	DDX_Control(pDX, IDC_PRACTAXRATE, m_nxeditPractaxrate);
	DDX_Control(pDX, IDC_PRACTAXRATE2, m_nxeditPractaxrate2);
	DDX_Control(pDX, IDC_PRACMAINNUM, m_nxeditPracmainnum);
	DDX_Control(pDX, IDC_PRACTOLLFREE, m_nxeditPractollfree);
	DDX_Control(pDX, IDC_PRACALTNUM, m_nxeditPracaltnum);
	DDX_Control(pDX, IDC_MODEM, m_nxeditModem);
	DDX_Control(pDX, IDC_PRACFAXNUM, m_nxeditPracfaxnum);
	DDX_Control(pDX, IDC_PRACEMAIL, m_nxeditPracemail);
	DDX_Control(pDX, IDC_OPENSUN, m_nxeditOpensun);
	DDX_Control(pDX, IDC_CLOSESUN, m_nxeditClosesun);
	DDX_Control(pDX, IDC_OPENMON, m_nxeditOpenmon);
	DDX_Control(pDX, IDC_CLOSEMON, m_nxeditClosemon);
	DDX_Control(pDX, IDC_OPENTUES, m_nxeditOpentues);
	DDX_Control(pDX, IDC_CLOSETUES, m_nxeditClosetues);
	DDX_Control(pDX, IDC_OPENWED, m_nxeditOpenwed);
	DDX_Control(pDX, IDC_CLOSEWED, m_nxeditClosewed);
	DDX_Control(pDX, IDC_OPENTHUR, m_nxeditOpenthur);
	DDX_Control(pDX, IDC_CLOSETHUR, m_nxeditClosethur);
	DDX_Control(pDX, IDC_OPENFRI, m_nxeditOpenfri);
	DDX_Control(pDX, IDC_CLOSEFRI, m_nxeditClosefri);
	DDX_Control(pDX, IDC_OPENSAT, m_nxeditOpensat);
	DDX_Control(pDX, IDC_CLOSESAT, m_nxeditClosesat);
	DDX_Control(pDX, IDC_PRACNOTES_BOX, m_nxeditPracnotesBox);
	DDX_Control(pDX, IDC_EDIT_PHARMACY_ID, m_btnEditPharmacyIDs);
	DDX_Control(pDX, IDC_EDIT_PHARMACY_STAFF, m_btnEditPharmacyStaff);
	DDX_Control(pDX, IDC_LOC_NPI_LABEL, m_nxstaticLocationNpiLabel);
	DDX_Control(pDX, IDC_LINK_WITH_DIRECTORY, m_checkLinkWithDirectory);
	DDX_Control(pDX, IDC_POS_LABEL, m_nxstaticPOSCodeLabel);
	DDX_Control(pDX, IDC_PRAC_TAXONOMY, m_nxeditTaxonomy);
	DDX_Control(pDX, IDC_PRAC_ABBREV, m_nxeditAbbreviation); 
	DDX_Control(pDX, IDC_ABBREV_LABEL, m_nxstaticAbbreviationLabel); 
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPracticeInfo, CNxDialog)
	//{{AFX_MSG_MAP(CPracticeInfo)
	ON_BN_CLICKED(IDC_ADDLOCATION, OnAddlocation)
	ON_BN_CLICKED(IDC_DELETELOCATION, OnDeletelocation)
	ON_BN_CLICKED(IDC_MANAGED, OnManaged)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_ADD_ALL, OnAddAll)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_REMOVE_ALL, OnRemoveAll)
	ON_BN_CLICKED(IDC_EDIT_PLACE_OF_SERVICE, OnEditPlaceOfService)
	ON_BN_CLICKED(IDC_PRACTICE_RIGHT, OnPracticeRight)
	ON_BN_CLICKED(IDC_PRACTICE_LEFT, OnPracticeLeft)
	ON_BN_CLICKED(IDC_LOCATION_ACTIVE, OnLocationActive)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT_FOR_NEW_PATIENTS, OnCheckDefaultForNewPatients)
	ON_BN_CLICKED(IDC_BUTTON_LOCATION_BIOGRAPHY, OnButtonLocationBiography)
	ON_BN_CLICKED(IDC_BUTTON_LOCATION_LOGO, OnButtonLocationLogo)
	ON_MESSAGE(WM_TABLE_CHANGED, OnTableChanged)
	ON_BN_CLICKED(IDC_BUTTON_LOCATION_IMAGE, OnButtonLocationImage)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_EDIT_PHARMACY_ID, &CPracticeInfo::OnBnClickedEditPharmacyId)
	ON_BN_CLICKED(IDC_EDIT_PHARMACY_STAFF, &CPracticeInfo::OnBnClickedEditPharmacyStaff)
	ON_BN_CLICKED(IDC_LINK_WITH_DIRECTORY, &CPracticeInfo::OnBnClickedLinkWithDirectory)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPracticeInfo message handlers

BEGIN_EVENTSINK_MAP(CPracticeInfo, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CPracticeInfo)
	ON_EVENT(CPracticeInfo, IDC_ALLOWED, 3 /* DblClickCell */, OnDblClickCellAllowed, VTS_I4 VTS_I2)
	ON_EVENT(CPracticeInfo, IDC_DISALLOWED, 3 /* DblClickCell */, OnDblClickCellDisallowed, VTS_I4 VTS_I2)
	ON_EVENT(CPracticeInfo, IDC_NXDATLISTPRACTICE, 16 /* SelChosen */, OnSelChosenLocation, VTS_I4)
	ON_EVENT(CPracticeInfo, IDC_NXDLPROVIDERS, 16 /* SelChosen */, OnSelChosenProvider, VTS_I4)
	ON_EVENT(CPracticeInfo, IDC_PLACEOFSERVICE_COMBO, 16 /* SelChosen */, OnSelChosenPlaceOfService, VTS_I4)
	ON_EVENT(CPracticeInfo, IDC_COORD_COMBO, 16 /* SelChosen */, OnSelChosenCoordinator, VTS_I4)
	ON_EVENT(CPracticeInfo, IDC_LOCATION_TYPE, 2 /* SelChanged */, OnSelChangedLocationType, VTS_DISPATCH VTS_DISPATCH)
	ON_EVENT(CPracticeInfo, IDC_LOCATION_TYPE, 1 /* SelChanging */, OnSelChangingLocationType, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CPracticeInfo, IDC_DEFAULT_LAB, 16 /* SelChosen */, OnSelChosenDefaultLab, VTS_DISPATCH)
	//}}AFX_EVENTSINK_MAP
	ON_EVENT(CPracticeInfo, IDC_LOCATION_DEFAULTCLAIMFORM, 16, CPracticeInfo::SelChosenLocationDefaultclaimform, VTS_DISPATCH)
END_EVENTSINK_MAP()

BOOL CPracticeInfo::OnInitDialog() 
{
	// (b.spivey, March 28, 2012) - PLID 47521 - Try/catch was never implemented here. 
	try {
		CNxDialog::OnInitDialog();
		m_changed = false;
		
		//get the control of the main datalist and initialize it 
		m_list = BindNxDataListCtrl(IDC_NXDATLISTPRACTICE);

		//get the control of the providers datalist and initialize it 
		m_provider = BindNxDataListCtrl(IDC_NXDLPROVIDERS);
		IRowSettingsPtr pRow = m_provider->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, "<No provider selected>");
		m_provider->InsertRow(pRow, 0);

		//coordinators
		m_coordinator = BindNxDataListCtrl(IDC_COORD_COMBO);
		pRow = m_coordinator->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, "<none>");
		pRow->PutValue(2, "<none>");
		pRow->PutValue(3, "<No employee selected>");
		m_coordinator->InsertRow(pRow, 0);

		m_placeOfService = BindNxDataListCtrl(IDC_PLACEOFSERVICE_COMBO);

		m_allowed = BindNxDataListCtrl(IDC_ALLOWED, false);
		m_disallowed = BindNxDataListCtrl(IDC_DISALLOWED, false);


		m_addBtn.AutoSet(NXB_RIGHT);
		m_remBtn.AutoSet(NXB_LEFT);
		m_addAllBtn.AutoSet(NXB_RRIGHT);
		m_remAllBtn.AutoSet(NXB_LLEFT);

		m_btnAdd.AutoSet(NXB_NEW);
		m_btnDelete.AutoSet(NXB_DELETE);

		// (z.manning, 04/25/2008) - PLID 29566 - Added more styles
		m_btnLogo.AutoSet(NXB_MODIFY);
		m_btnImage.AutoSet(NXB_MODIFY);
		m_btnBiography.AutoSet(NXB_MODIFY);

		//DRT 11/19/2008 - PLID 32081
		m_btnEditPharmacyStaff.AutoSet(NXB_MODIFY);
		m_btnEditPharmacyIDs.AutoSet(NXB_MODIFY);

		//set the current selection to be zero 
		m_list->CurSel = m_CurSelect = 0;

		m_btnPracticeRight.AutoSet(NXB_RIGHT);
		m_btnPracticeLeft.AutoSet(NXB_LEFT);
		EnsureButtons();
		
		m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true); 
		m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);


		//types
		//TES 11/13/2007 - PLID 28059 - This is a datalist2, not a datalist.
		m_pTypeList = BindNxDataList2Ctrl(IDC_LOCATION_TYPE, false);

		NXDATALIST2Lib::IRowSettingsPtr pTypeRow;
		pTypeRow = m_pTypeList->GetNewRow();

		pTypeRow->PutValue(0, (long) 1);
		pTypeRow->PutValue(1, _variant_t("General"));
		m_pTypeList->AddRowAtEnd(pTypeRow, NULL);
			
		pTypeRow = m_pTypeList->GetNewRow();
		pTypeRow->PutValue(0, (long)2);
		pTypeRow->PutValue(1, _variant_t("Lab"));;
		m_pTypeList->AddRowAtEnd(pTypeRow, NULL);

		pTypeRow = m_pTypeList->GetNewRow();
		pTypeRow->PutValue(0, (long)3);
		pTypeRow->PutValue(1, _variant_t("Pharmacy"));;
		m_pTypeList->AddRowAtEnd(pTypeRow, NULL);

		// (z.manning 2010-01-11 10:43) - PLID 24044 - Default lab option
		m_pdlDefaultLab = BindNxDataList2Ctrl(this, IDC_DEFAULT_LAB, GetRemoteData(), true);
		// (s.tullis 2016-03-07 11:56) - PLID 68444 - Default Claim Form
		m_pdlDefaultClaimForm = BindNxDataList2Ctrl(this, IDC_LOCATION_DEFAULTCLAIMFORM, GetRemoteData(), false);
		InitClaimFormList();

		((CNxEdit*)GetDlgItem(IDC_NAME))->SetLimitText(255);
		((CNxEdit*)GetDlgItem(IDC_PRACADDRESS1))->SetLimitText(75);
		((CNxEdit*)GetDlgItem(IDC_PRACADDRESS2))->SetLimitText(75);
		((CNxEdit*)GetDlgItem(IDC_PRACZIP))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_PRACCITY))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_PRACSTATE))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_WEBSITE))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_PRAC_NPI))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_PRACEMAIL))->SetLimitText(50);
		((CNxEdit*)GetDlgItem(IDC_PRACNOTES_BOX))->SetLimitText(255);
		// (j.jones 2012-03-23 14:23) - PLID 42388 - added taxonomy code
		m_nxeditTaxonomy.SetLimitText(20);
		// (b.spivey, March 27, 2012) - PLID 47521 - Cap at 1,000 characters. I know it's an abbreviation-- lets be flexible anyways. 
		m_nxeditAbbreviation.SetLimitText(1000); 
		// (j.fouts 2013-06-07 09:20) - PLID 57047 - Removed SPI Root from Locations Tab
	} NxCatchAll(__FUNCTION__); 
	return TRUE;
}

void CPracticeInfo::UpdateView(bool bForceRefresh) // (a.walling 2010-10-12 15:27) - PLID 40906 - UpdateView with option to force a refresh
{	
	// (z.manning, 5/19/2006, PLID 20726) - We may still have focus on a field that has been changed,
	// so let's kill it's focus to ensure that no changes are lost.
	// (a.walling 2010-10-12 17:43) - PLID 40908 - Forces focus lost messages (now part of CNexTechDialog)
	CheckFocus();

	//(e.lally 2009-03-25) PLID 33634 - Update label for standard/OHIP.
	// (j.jones 2010-11-03 15:56) - PLID 39620 - supported Alberta HLINK
	if(!UseOHIP() && !UseAlbertaHLINK()) {
		m_nxstaticLocationNpiLabel.SetWindowText("NPI");
	}
	else{
		m_nxstaticLocationNpiLabel.SetWindowText("Facility Number");
	}

	// (j.jones 2012-03-21 11:38) - PLID 48155 - if the Alberta preference is on,
	// rename the POS field to Functional Centre
	if(UseAlbertaHLINK()) {
		m_nxstaticPOSCodeLabel.SetWindowText("Functional Centre");
	}
	else {
		m_nxstaticPOSCodeLabel.SetWindowText("POS Code");
	}

	//Load all the information onto the page
	if (m_ProviderChecker.Changed())
	{
		// The load() function will fill in the default
		// provider.
		m_provider->Requery();
		IRowSettingsPtr pRow = m_provider->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, "<No provider selected>");
		m_provider->InsertRow(pRow, 0);
	}
	if (m_PatCoordChecker.Changed())
	{
		// The load() function will fill in the default
		// coordinator.
		m_coordinator->Requery();
		IRowSettingsPtr pRow = m_coordinator->GetRow(-1);
		pRow->PutValue(0, (long)-1);
		pRow->PutValue(1, "<none>");
		pRow->PutValue(2, "<none>");
		pRow->PutValue(3, "<No employee selected>");
		m_coordinator->InsertRow(pRow, 0);
	}

	// (j.gruber 2009-10-06 17:02) - PLID 35826 - reset here in case they changed the preference
	m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");

	if (m_bLookupByCity) {
		ChangeZOrder(IDC_PRACZIP, IDC_PRACSTATE);
	} else {
		ChangeZOrder(IDC_PRACZIP, IDC_PRACADDRESS2);
	}

	Load();
}


void CPracticeInfo::SetDlgItemTime(int nID, const COleVariant &var)
{//needed since setdlgitemvar defaults to date only
	//check that its a date
	if (var.vt == VT_DATE) {
		SetDlgItemText(nID, FormatDateTimeForInterface(VarDateTime(var),DTF_STRIP_SECONDS, dtoTime));
	}
	//if its not a date it is not open, so enter closed 
	else {
		SetDlgItemText(nID, "CLOSED");
	}
}


void CPracticeInfo::LoadUsers()
{
	_RecordsetPtr	rs;
	IRowSettingsPtr pRow = NULL;
	long			id;

	if (m_list->CurSel == -1)
		return;

	id = m_list->Value[m_list->CurSel][0].lVal;

	m_disallowed->Clear();
	m_allowed->Clear();

	if (m_managed.GetCheck())
	{
		rs = CreateRecordset(
			"SELECT PersonID, UserName, 1 AS Allowed, Administrator "
			"FROM UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID AND UsersT.PersonID > 0 "
			"WHERE PersonT.Archived = 0 AND PersonID IN ("
				"SELECT PersonID "
				"FROM UserLocationT "
				"WHERE LocationID = %i "
			") "
				
			"UNION ALL "

			"SELECT PersonID, UserName, 0 AS Allowed, Administrator "
			"FROM UsersT INNER JOIN PersonT ON UsersT.PersonID = PersonT.ID AND UsersT.PersonID > 0 "
			"WHERE PersonT.Archived = 0 AND PersonID NOT IN ("
				"SELECT PersonID "
				"FROM UserLocationT "
				"WHERE LocationID = %i "
			")", id, id);
	
		while (!rs->eof)
		{	if (rs->Fields->Item["Allowed"]->Value.lVal)
			{	pRow = m_allowed->GetRow(-1);
				pRow->PutValue(0, rs->Fields->Item["PersonID"]->Value);
				pRow->PutValue(1, rs->Fields->Item["UserName"]->Value);
				pRow->PutValue(2, rs->Fields->Item["Administrator"]->Value);
				m_allowed->AddRow(pRow);
			}
			else
			{	pRow = m_disallowed->GetRow(-1);
				pRow->PutValue(0, rs->Fields->Item["PersonID"]->Value);
				pRow->PutValue(1, rs->Fields->Item["UserName"]->Value);
				pRow->PutValue(2, rs->Fields->Item["Administrator"]->Value);
				m_disallowed->AddRow(pRow);
			}
			rs->MoveNext();
		}
		rs->Close();
	}
}

void CPracticeInfo::Load() 
{
	_RecordsetPtr		rs;
	long				id;
	double				tax1,tax2;
	CString				str;
	ADODB::FieldsPtr	fields;

	try
	{
		if (m_list->CurSel == -1) {
			m_list->CurSel = 0;
			if(m_list->CurSel == -1)
				return;
		}

		id = m_list->Value[m_list->CurSel][0].lVal;

		// (a.walling 2009-03-30 09:52) - PLID 33729 - Support Link To SureScripts Directory
		// (z.manning 2010-01-11 10:45) - PLID 24044 - DefaultLabID
		// (j.jones 2012-03-23 14:23) - PLID 42388 - added taxonomy code
		// (b.spivey, March 27, 2012) - PLID 47521 - Added Abbreviation. 
		// (j.fouts 2013-06-07 09:20) - PLID 57047 - Removed SPI Root from Locations Tab
		// (s.tullis 2016-03-07 11:56) - PLID 68444 
		rs = CreateParamRecordset("SELECT ID, Managed, Name, Address1, Address2, City, State, "
			"Zip, Phone, Phone2, TollFreePhone, Fax, POSID, OnLineAddress, EIN, NPI, TaxRate, TaxRate2, "
			"DefaultProviderID, DefaultCoordinatorID, ModemPhone, Website, Notes, "
			"SundayOpen, SundayClose, MondayOpen, MondayClose, TuesdayOpen, TuesdayClose, "
			"WednesdayOpen, WednesdayClose, ThursdayOpen, ThursdayClose, "
			"FridayOpen, FridayClose, SaturdayOpen, SaturdayClose, Active, IsDefault, TypeID, LinkToDirectory, "
			"DefaultLabID, TaxonomyCode, LocationAbbreviation, DefaultClaimForm "
			"FROM LocationsT WHERE ID = {INT}", id);

		// (a.walling 2016-02-15 09:48) - PLID 67827 - Don't crash if the location was deleted underneath CPracticeInfo dialog
		if (rs->eof) {
			MessageBox("This location is no longer available.");
			m_list->Requery();
			m_list->WaitForRequery(NXDATALISTLib::dlPatienceLevelWaitIndefinitely);
			if (m_list->GetRowCount()) {
				m_list->CurSel = 0;
			}
			return;
		}

		fields = rs->Fields;

		//Set all except time Fields
		SetDlgItemVar(IDC_NAME, m_list->Value[m_list->CurSel][1],				true, true);

		SetDlgItemVar(IDC_PRACADDRESS1, fields->Item["Address1"]->Value,		true, true);
		SetDlgItemVar(IDC_PRACADDRESS2, fields->Item["Address2"]->Value,		true, true);
		SetDlgItemVar(IDC_PRACCITY,		fields->Item["City"]->Value,			true, true);
		SetDlgItemVar(IDC_PRACSTATE,	fields->Item["State"]->Value,			true, true);
		SetDlgItemVar(IDC_PRACZIP,		fields->Item["Zip"]->Value,				true, true);
		SetDlgItemVar(IDC_PRACEIN,		fields->Item["EIN"]->Value,				true, true);
		SetDlgItemVar(IDC_PRAC_NPI,		fields->Item["NPI"]->Value,				true, true);
		SetDlgItemVar(IDC_PRACEMAIL,	fields->Item["OnLineAddress"]->Value,	true, true);
		SetDlgItemVar(IDC_PRACMAINNUM,	fields->Item["Phone"]->Value,			true, true);
		SetDlgItemVar(IDC_PRACALTNUM,	fields->Item["Phone2"]->Value,			true, true);
		SetDlgItemVar(IDC_PRACFAXNUM,	fields->Item["Fax"]->Value,				true, true);
		SetDlgItemVar(IDC_PRACTOLLFREE, fields->Item["TollFreePhone"]->Value,	true, true);
		SetDlgItemVar(IDC_PRACNOTES_BOX,fields->Item["Notes"]->Value,			true, true);
		SetDlgItemVar(IDC_MODEM,		fields->Item["ModemPhone"]->Value,		true, true);
		SetDlgItemVar(IDC_WEBSITE,		fields->Item["Website"]->Value,			true, true);
		
		// (j.jones 2012-03-23 14:23) - PLID 42388 - added taxonomy code
		m_strPrevTaxonomyCode = AdoFldString(fields, "TaxonomyCode", "");
		SetDlgItemText(IDC_PRAC_TAXONOMY, m_strPrevTaxonomyCode);
		
		// (a.walling 2008-05-19 18:00) - PLID 27810 - Cache the loaded NPI
		m_strOriginalNPI = AdoFldString(fields, "NPI", "");

		// (b.spivey, March 27, 2012) - PLID 47521 - Get the location abbreviation out of the database, save it for auditing. 
		m_strPrevAbbrev = AdoFldString(fields, "LocationAbbreviation", ""); 
		SetDlgItemText(IDC_PRAC_ABBREV,	m_strPrevAbbrev); 

		// (j.fouts 2013-06-07 09:20) - PLID 57047 - Removed SPI Root from Locations Tab
		if (rs->Fields->Item["Managed"]->Value.boolVal)
			m_managed.SetCheck(TRUE);
		else m_managed.SetCheck(FALSE);

		if (rs->Fields->Item["Active"]->Value.boolVal)
			m_buActive.SetCheck(TRUE);
		else m_buActive.SetCheck(FALSE);

		if (rs->Fields->Item["IsDefault"]->Value.boolVal)
			m_checkDefaultForNewPatients.SetCheck(TRUE);
		else m_checkDefaultForNewPatients.SetCheck(FALSE);

		long nID = AdoFldLong(rs, "POSID", -1);
		m_placeOfService->SetSelByColumn(0, nID);

		m_provider->SetSelByColumn(0, rs->Fields->Item["DefaultProviderID"]->Value);
		m_coordinator->SetSelByColumn(0, rs->Fields->Item["DefaultCoordinatorID"]->Value);

		//Set all other Fields
		//SetDlgItemTime checks variants
		SetDlgItemTime (IDC_OPENSUN,	 fields->Item["SundayOpen"]->Value);
		SetDlgItemTime (IDC_CLOSESUN,	 fields->Item["SundayClose"]->Value);
		SetDlgItemTime (IDC_OPENMON,	 fields->Item["MondayOpen"]->Value);
		SetDlgItemTime (IDC_CLOSEMON,	 fields->Item["MondayClose"]->Value);
		SetDlgItemTime (IDC_OPENTUES,	 fields->Item["TuesdayOpen"]->Value);
		SetDlgItemTime (IDC_CLOSETUES,	 fields->Item["TuesdayClose"]->Value);
		SetDlgItemTime (IDC_OPENWED,	 fields->Item["WednesdayOpen"]->Value);
		SetDlgItemTime (IDC_CLOSEWED,	 fields->Item["WednesdayClose"]->Value);
		SetDlgItemTime (IDC_OPENTHUR,	 fields->Item["ThursdayOpen"]->Value);
		SetDlgItemTime (IDC_CLOSETHUR,	 fields->Item["ThursdayClose"]->Value);
		SetDlgItemTime (IDC_OPENFRI,	 fields->Item["FridayOpen"]->Value);
		SetDlgItemTime (IDC_CLOSEFRI,	 fields->Item["FridayClose"]->Value);
		SetDlgItemTime (IDC_OPENSAT,	 fields->Item["SaturdayOpen"]->Value);
		SetDlgItemTime (IDC_CLOSESAT,	 fields->Item["SaturdayClose"]->Value);

		//check to see how big the tax rate number is and format it accordingly
		tax1 = AdoFldDouble(fields->Item["TaxRate"]);
		tax1 = (tax1 - 1.0) * 100.0;
		str.Format("%0.04f%%", tax1);
		SetDlgItemText (IDC_PRACTAXRATE, str);

		tax2 = AdoFldDouble(fields->Item["TaxRate2"]);
		tax2 = (tax2 - 1.0) * 100.0;
		str.Format("%0.04f%%", tax2);
		SetDlgItemText (IDC_PRACTAXRATE2, str);

		//PLID 21089 - add type to locations
		//DRT 11/19/2008 - PLID 32081 - Hide or show type specific fields
		long nTypeID = AdoFldLong(rs, "TypeID");
		m_pTypeList->SetSelByColumn(0, nTypeID);
		
		// (a.walling 2009-03-30 09:52) - PLID 33729 - Support Link To SureScripts Directory
		if (nTypeID == 3) {
			_variant_t varLinked = rs->Fields->Item["LinkToDirectory"]->Value;

			if (varLinked.vt == VT_BOOL) {
				m_checkLinkWithDirectory.SetCheck(VarBool(varLinked));
			} else {
				// just set it as TRUE, it will collapse to either TRUE or FALSE in data during the next update.
				m_checkLinkWithDirectory.SetCheck(TRUE);
				//m_checkLinkWithDirectory.SetCheck(BST_INDETERMINATE);
			}
		} else {
			m_checkLinkWithDirectory.SetCheck(FALSE);
		}

		// (z.manning 2010-01-11 10:47) - PLID 24044 - Default lab ID
		m_pdlDefaultLab->WaitForRequery(NXDATALIST2Lib::dlPatienceLevelWaitIndefinitely);
		m_pdlDefaultLab->SetSelByColumn(dlccID, fields->GetItem("DefaultLabID")->GetValue());
		// (s.tullis 2016-03-07 11:56) - PLID 68444 - Default Claim Form
		InitClaimFormList();
		ShowHideDefaultClaimFormControls(m_managed.GetCheck() == BST_CHECKED ? true : false);
		long nClaimFormID = AdoFldLong(rs, "DefaultClaimForm");
		m_pdlDefaultClaimForm->SetSelByColumn(cflID, nClaimFormID);

		EnsureTypeSpecificElements(nTypeID);
		

		rs->Close();
		
		EnsureButtons();

		LoadUsers();

	}NxCatchAll("Could not load practice information");

	m_changed = false;
}

	
void CPracticeInfo::Save (int nID)
{
	CString field, value, strOldLoc, strOldTaxonomy;	//strOldLoc for auditing
	if(m_list->GetCurSel()==-1)
		return;

	try{
		//set the field names corresponding to each control
		m_changed = false;
		switch (nID)
		{	case IDC_NAME:
			{	GetDlgItemText(nID, value);
				value.TrimRight();
				strOldLoc = CString(m_list->GetValue(m_list->CurSel, 1).bstrVal);

				if(strOldLoc.CompareNoCase(value) != 0) {
					//different name
					CString strWarn;
					// (j.gruber 2007-08-07 11:20) - PLID 26966 - included that this could change many things in the program
					strWarn.Format("You are about to change the name of this location from '%s' to '%s'.\n"
						"This will change the name of this location in many places in Practice.\n"
						"Are you sure you wish to do this?",strOldLoc,value);
					if(IDNO == MessageBox(strWarn,"Practice",MB_ICONEXCLAMATION|MB_YESNO)) {
						SetDlgItemText(IDC_NAME, strOldLoc);
						return;
					}
				}

				// JJ 7/8/01 /////////////////////////
				if(!IsRecordsetEmpty("SELECT Name FROM LocationsT WHERE Name = '%s' AND ID <> %li", _Q(value),m_list->GetValue(m_list->GetCurSel(),0).lVal))
				{
					MessageBox("There is already a location with this name. Please choose another name.","Practice",MB_OK|MB_ICONEXCLAMATION);
					SetDlgItemVar(IDC_NAME, m_list->Value[m_list->CurSel][1],true,true);
					return;
				}
				///////////////////////////////////////

				m_list->Value[m_list->CurSel][1] = _bstr_t(value);

				if(m_list->GetValue(m_list->GetCurSel(),0).lVal == GetCurrentLocationID()) {
					SetCurrentLocationName(value);
					GetMainFrame()->LoadTitleBarText();
				}
				
				field = "Name";
				break;
			}
			case IDC_PRACADDRESS1: 
				field = "Address1";
				break;
			case IDC_PRACADDRESS2: 
				field = "Address2";
				break;
			case IDC_PRACCITY: 
				field = "City";
				break;
			case IDC_PRACSTATE: 
				field = "State";
				break;
			case IDC_PRACZIP: 
				field = "Zip";
				break;
			case IDC_PRACEIN: 
				field = "EIN";
				break;
			case IDC_PRAC_NPI: 
				field = "NPI";
				// (a.walling 2008-05-19 16:15) - PLID 27810 - Check for valid NPI
				GetDlgItemText(nID, value);
				value.TrimRight();
				if (m_strOriginalNPI != value) {
					CheckNPI(value, this);
					m_strOriginalNPI = value;
				}
				break;
			// (j.jones 2012-03-23 14:23) - PLID 42388 - added taxonomy code
			case IDC_PRAC_TAXONOMY: 
				field = "TaxonomyCode";
				break;
			// (b.spivey, March 27, 2012) - PLID 47521 - added abbreviation. 
			case IDC_PRAC_ABBREV:
				field = "LocationAbbreviation";
				break; 
			case IDC_PRACEMAIL: 
				field = "OnLineAddress";
				break;
			case IDC_PRACMAINNUM: 
				field = "Phone";
				break;
			case IDC_PRACALTNUM: 
				field = "Phone2";
				break;
			case IDC_PRACTOLLFREE:
				field = "TollFreePhone";
				break;
			case IDC_PRACFAXNUM: 
				field = "Fax";
				break;
			case IDC_OPENSUN:		
				field = "SundayOpen";
				break;
			case IDC_CLOSESUN: 
				field = "SundayClose";
				break;
			case IDC_OPENMON:
				field = "MondayOpen";
				break;
			case IDC_CLOSEMON: 
				field = "MondayClose";
				break;
			case IDC_OPENTUES: 
				field = "TuesdayOpen";
				break;
			case IDC_CLOSETUES: 
				field = "TuesdayClose";
				break;
			case IDC_OPENWED: 
				field = "WednesdayOpen";
				break;
			case IDC_CLOSEWED: 
				field = "WednesdayClose";
				break;
			case IDC_OPENTHUR: 
				field = "ThursdayOpen";
				break;
			case IDC_CLOSETHUR: 
				field = "ThursdayClose";
				break;
			case IDC_OPENFRI:
				field = "FridayOpen";
				break;
			case IDC_CLOSEFRI: 
				field = "FridayClose";
				break;
			case IDC_OPENSAT:		
				field = "SaturdayOpen";
				break;
			case IDC_CLOSESAT: 
				field = "SaturdayClose";
				break;
			case IDC_PRACNOTES_BOX:
				field = "Notes";
				break;
			case IDC_MODEM:
				field = "ModemPhone";
				break;
			case IDC_WEBSITE:
				field = "Website";
				break;
			// (j.fouts 2013-06-07 09:20) - PLID 57047 - Removed SPI Root from Locations Tab
			case IDC_PRACTAXRATE:
			{	
				CString cstax;
				CString str;
				double tax;

				//get the text from the dialog			
				GetDlgItemText (nID, value);
				//only allow it to include these numbers or a "."
				cstax = Include(value, ".1234567890");
				tax = (atof(cstax.GetBuffer(cstax.GetLength())) / 100.0) + 1.0;

				//DRT 7/6/2007 - PLID 25210 - We need to constrain the rax rate to 0 - 100%.  If they set it to something out of that range, just reset it to 0%
				if(tax < 1.0) {
					tax = 1.0;
				}
				else if(tax > 2.0) {
					tax = 2.0;
				}

				value.Format("%0.09g", tax);
				_variant_t varCurSelVal;
				//set varCurSel equal to the current selection in the data list
				varCurSelVal = m_list->GetValue(m_list->GetCurSel(), 0);
				//check to see that it is a long becuase if it isn't then we need to give an error
				if (varCurSelVal.vt == VT_I4)
				{	ExecuteSql("Update LocationsT SET LocationsT.TaxRate = %s WHERE LocationsT.ID = %li", value, varCurSelVal.lVal);
					CClient::RefreshTable(NetUtils::Tax, varCurSelVal.lVal);
					tax = (tax - 1.0) * 100.0;
					str.Format("%0.04f%%", tax);
					SetDlgItemText (IDC_PRACTAXRATE, str);
				}
				else AfxMessageBox("Could not save Taxrate 1, Invalid Location Selection.");
				return;
			}
			case IDC_PRACTAXRATE2:
			{	
				CString cstax;
				CString str;
				double tax;

				//get the text from the dialog			
				GetDlgItemText (nID, value);
				//only allow it to include these numbers or a "."
				cstax = Include(value, ".1234567890");
				tax = (atof(cstax.GetBuffer(cstax.GetLength())) / 100.0) + 1.0;

				//DRT 7/6/2007 - PLID 25210 - We need to constrain the rax rate to 0 - 100%.  If they set it to something out of that range, just reset it to 0%
				if(tax < 1.0) {
					tax = 1.0;
				}
				else if(tax > 2.0) {
					tax = 2.0;
				}

				value.Format("%0.09g", tax);
				_variant_t varCurSelVal;
				//set varCurSel equal to the current selection in the data list
				varCurSelVal = m_list->GetValue(m_list->GetCurSel(), 0);
				//check to see that it is a long becuase if it isn't then we need to give an error
				if (varCurSelVal.vt == VT_I4)
				{	ExecuteSql("Update LocationsT SET LocationsT.TaxRate2 = %s WHERE LocationsT.ID = %li", value, varCurSelVal.lVal);
					CClient::RefreshTable(NetUtils::Tax, varCurSelVal.lVal);
					tax = (tax - 1.0) * 100.0;
					str.Format("%0.04f%%", tax);
					SetDlgItemText (IDC_PRACTAXRATE2, str);
				}
				else AfxMessageBox("Could not save Taxrate 2, Invalid Location Selection.");
				return;
			}
			default:
				return;
		}
		GetDlgItemText(nID, value);
		value.TrimRight();
		_variant_t varCurSelVal;
		//get the id of the current selection out of the datalist
		varCurSelVal = m_list->GetValue(m_list->GetCurSel(), 0);
		//check that we have a long because if we don't then we can't use it,
		//so we need to give an error
		if (varCurSelVal.vt == VT_I4) {
			CSqlTransaction sqlTran;
			sqlTran.Begin();
			ExecuteSql("UPDATE LocationsT SET LocationsT.%s = '%s' WHERE LocationsT.ID = %li", field, _Q(value), varCurSelVal.lVal );
			//auditing
			if(field == "Name" && value != strOldLoc) {
				//TES 8/9/2011 - PLID 44862 - Need to update the user-defined permission (note that this will have no effect if this is a non-general location)
				long nSecurityObjectID = UpdateUserDefinedPermissionNameInData(bioLabLocations, varCurSelVal.lVal, value);
				sqlTran.Commit();
				UpdateUserDefinedPermissionNameInMemory(nSecurityObjectID, value);
				long nAuditID = BeginNewAuditEvent();
				AuditEvent(-1, "", nAuditID, aeiLocationName, -1, strOldLoc, value, aepMedium, aetChanged);
			}
			else {
				sqlTran.Commit();
			}

			// (j.jones 2012-03-23 14:23) - PLID 42388 - audit the taxonomy code if it changed (case-sensitive)
			if(nID == IDC_PRAC_TAXONOMY) {
				if(m_strPrevTaxonomyCode != value) {
					long nAuditID = BeginNewAuditEvent();
					CString strLocationName = VarString(m_list->GetValue(m_list->CurSel, 1));
					AuditEvent(-1, "", nAuditID, aeiLocationTaxonomyCode, varCurSelVal.lVal, strLocationName + ": " + m_strPrevTaxonomyCode, value, aepLow, aetChanged);
				}
				m_strPrevTaxonomyCode = value;
			}
			
			// (b.spivey, March 28, 2012) - PLID 47521 - Audit the abbreviation if it was changed. 
			if(nID == IDC_PRAC_ABBREV) {
				if(m_strPrevAbbrev != value) {
					CString strLocationName = VarString(m_list->GetValue(m_list->CurSel, 1));
					long nAuditID = BeginNewAuditEvent(); 
					AuditEvent(-1, strLocationName, nAuditID, aeiLocationAbbreviation, varCurSelVal.lVal, m_strPrevAbbrev, value, aepLow, aetChanged);
				}
				m_strPrevAbbrev = value; 
			}

			// (a.walling 2007-08-06 12:36) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
			CClient::RefreshTable(NetUtils::LocationsT, varCurSelVal.lVal);
		}
		else AfxMessageBox("Could not update location, invalid selection.");
	}NxCatchAll("Could not update location");
}


void CPracticeInfo::FormatDlgItem(UINT nId)
{
	CString str;
	switch (nId) {
	case IDC_PRACNAME: 
	case IDC_PRACADDRESS1: 
	case IDC_PRACADDRESS2: 
	case IDC_PRACCITY:
	case IDC_PRACSTATE:
		// For these fields we simply want to capitalize the current text
		Capitalize(nId);
		break;
	case IDC_PRACEIN: 
		// Format like a good EIN
		FormatItem(nId, "##-#######");
		break;
	case IDC_PRACMAINNUM:
	case IDC_PRACALTNUM:
	case IDC_PRACFAXNUM:
	case IDC_PRACTOLLFREE:
	case IDC_MODEM:
		// Format like a phone number
		GetDlgItemText(nId, str);
		str.TrimRight();
		if (str != "") {
			if(m_bFormatPhoneNums) {
				FormatItem (nId, m_strPhoneFormat);
			}
		}
		break;
		break;
	case IDC_PRACTAXRATE:
	case IDC_PRACTAXRATE2:
		// Format like a tax rate
//		FormatItem (nId, "##.###%");
		break;
	case IDC_PRACZIP:
	case IDC_PLACEOFSERVICE:
	case IDC_PRAC_NPI:
	// (j.jones 2012-03-23 14:23) - PLID 42388 - added taxonomy code
	case IDC_PRAC_TAXONOMY:
	// (b.spivey, March 28, 2012) - PLID 47521 - added Abbreviation. 
	case IDC_PRAC_ABBREV:
	default:
		// No special formating
		m_changed = true;
		break;
	}
}

bool CPracticeInfo::PreKillFocusZipCode()
{
	// (j.gruber 2009-10-07 17:25) - PLID 35826 - updated for city lookup
	if (!m_bLookupByCity) {

		CString city, 
				state,
				tempZip,
				tempCity,
				tempState,
				value;
		GetDlgItemText(IDC_PRACZIP, value);
		GetDlgItemText(IDC_PRACCITY, tempCity);
		GetDlgItemText(IDC_PRACSTATE, tempState);
		bool bSaveCity = false;
		bool bSaveState = false;
		tempCity.TrimRight();
		tempState.TrimRight();
		// (d.thompson 2009-08-24) - PLID 31136 - Prompt to see if they wish to overwrite city/state
		if(!tempCity.IsEmpty() || !tempState.IsEmpty()) {
			MAINTAIN_FOCUS(); // (a.walling 2011-08-26 09:56) - PLID 45199 - Safely maintain focus
			if(AfxMessageBox("You have changed the postal code but the city or state already have data in them.  Would you like to overwrite "
				"this data with that of the new postal code?", MB_YESNO) == IDYES)
			{
				//Just treat them as empty and the code below will fill them.
				tempCity.Empty();
				tempState.Empty();
			}
		}
		if(tempCity == "" || tempState == "") {
			GetZipInfo(value, &city, &state);
			// (s.tullis 2013-10-21 10:17) - PLID 45031 - If 9-digit zipcode match fails compair it with the 5-digit zipcode.
			if(city == "" && state == ""){					
				tempZip = value.Left(5);// Get the 5 digit zip code
				GetZipInfo(tempZip, &city, &state);
				// (b.savon 2014-04-03 13:02) - PLID 61644 - If you enter a 9
				//digit zipcode in the locations tab of Administrator, it looks
				//up the city and state based off the 5 digit code, and then 
				//changes the zip code to 5 digits. It should not change the zip code.
			}
			if(tempCity == "") {
				bSaveCity = true;
				SetDlgItemText(IDC_PRACCITY, city);
			}
			else city = tempCity;
			if(tempState == "") {
				SetDlgItemText(IDC_PRACSTATE, state);
				bSaveState = true;
			}
			else state = tempState;
		}
		// Now save the zip, city and state
		// Get the currently selected location from the datalist
		if(m_list->GetCurSel()==-1)
			return false;
		_variant_t varCurSelId = m_list->GetValue(m_list->GetCurSel(), 0);
		if (varCurSelId.vt == VT_I4) {
			try {
				// Execute an update query to modify this location's record
				ExecuteSql(
				"UPDATE LocationsT SET "
				"LocationsT.Zip = '%s'%s%s "
				"WHERE LocationsT.ID = %li", _Q(value), 
				(bSaveCity ? (", LocationsT.City = '" + city + "'") : ""),
				(bSaveState ? (", LocationsT.State = '" + state + "'") : ""), varCurSelId.lVal);
				
				//DRT 3/11/03 - This is idiotic.  We're requerying the entire screen because the zip code changed - but
				//		we already set the changed fields above!

				// Requery the screen to reflect the changes in the data
				//Load();

				return true;
			} catch (_com_error e) {
				// Unexpected com error
				CString strError;
				strError.Format("%s  ZipCode info not saved.", (char*) e.Description());
				MsgBox(MB_ICONEXCLAMATION|MB_OK, strError);
				return false;
			}
		} else {
			// The value we got from the datalist wasn't what we were expecting
			CString strErr;
			HandleException(NULL, _T("PreKillFocusZipCode Error 1.  ZipCode info not saved."), __LINE__, __FILE__);
			return false;
		}
	}
	else {
		return false;
	}
}

// (j.gruber 2009-10-07 17:26) - PLID 35826 - for city lookup
bool CPracticeInfo::PreKillFocusCity()
{
	try {
		// (j.gruber 2009-10-07 17:25) - PLID 35826 - updated for city lookup
		if (m_bLookupByCity) {
			CString zip, 
					state,
					tempZip,
					tempState,
					value;
			GetDlgItemText(IDC_PRACCITY, value);
			GetDlgItemText(IDC_PRACZIP, tempZip);
			GetDlgItemText(IDC_PRACSTATE, tempState);
			bool bSaveZip = false;
			bool bSaveState = false;
			bool bZipCheck= false;
			tempZip.TrimRight();
			tempState.TrimRight();
			// (d.thompson 2009-08-24) - PLID 31136 - Prompt to see if they wish to overwrite city/state
			if(!tempZip.IsEmpty() || !tempState.IsEmpty()) {
				MAINTAIN_FOCUS(); // (a.walling 2011-08-26 09:56) - PLID 45199 - Safely maintain focus
				if(AfxMessageBox("You have changed the city but the postal code or state already have data in them.  Would you like to overwrite "
					"this data with that of the new city?", MB_YESNO) == IDYES)
				{
					//Just treat them as empty and the code below will fill them.
					tempZip.Empty();
					tempState.Empty();
				}
			}
			if(tempZip == "" || tempState == "") {
				GetCityInfo(value, &zip, &state);
				if(tempZip == "") {
					bSaveZip = true;
					SetDlgItemText(IDC_PRACZIP, zip);
				}
				else zip = tempZip;
				if(tempState == "") {
					SetDlgItemText(IDC_PRACSTATE, state);
					bSaveState = true;
				}
				else state = tempState;
			}
			// Now save the zip, city and state
			// Get the currently selected location from the datalist
			if(m_list->GetCurSel()==-1)
				return false;
			_variant_t varCurSelId = m_list->GetValue(m_list->GetCurSel(), 0);
			if (varCurSelId.vt == VT_I4) {
				try {
					// Execute an update query to modify this location's record
					ExecuteSql(
						"UPDATE LocationsT SET "
						"LocationsT.City = '%s'%s%s "
						"WHERE LocationsT.ID = %li", _Q(value), 
						(bSaveZip ? (", LocationsT.Zip = '" + zip + "'") : ""),
						(bSaveState ? (", LocationsT.State = '" + state + "'") : ""), varCurSelId.lVal);
					
					//DRT 3/11/03 - This is idiotic.  We're requerying the entire screen because the zip code changed - but
					//		we already set the changed fields above!

					// Requery the screen to reflect the changes in the data
					//Load();

					return true;
				} catch (_com_error e) {
					// Unexpected com error
					CString strError;
					strError.Format("%s  City ZipCode info not saved.", (char*) e.Description());
					MsgBox(MB_ICONEXCLAMATION|MB_OK, strError);
					return false;
				}
			} else {
				// The value we got from the datalist wasn't what we were expecting
				CString strErr;
				HandleException(NULL, _T("PreKillFocusCity Error 1.  ZipCode info not saved."), __LINE__, __FILE__);
				return false;
			}
		}
		else {
			return false;
		}
	}NxCatchAll(__FUNCTION__);

	return false;
}


bool CPracticeInfo::PreKillFocusOfficeHours(UINT nId)
{	
	// This function can only be called once at a time (no concurrent execution)
	static bool bOfficeHoursRunning = false;
	if (!bOfficeHoursRunning) {
		bOfficeHoursRunning = true;
		
		// First calculate the necessary info based on the given officehours control ID
		UINT nPartnerId = GetCtrlPartnerId(nId);
		CString strFieldName = GetCtrlFieldName(nId);
		CString strPartnerName = GetCtrlFieldName(nPartnerId);

		// Get the given officehours value as a date/time
		COleDateTime dtGivenTime;
		CString strGivenTime;
		GetDlgItemText(nId, strGivenTime);
		dtGivenTime.ParseDateTime(strGivenTime, VAR_TIMEVALUEONLY);
		
		bool bDoSavePartner = false;
		COleDateTime dtToBeSaved, dtPartnerToBeSaved;

		// Get the current selection
		long nCurSel = m_list->GetCurSel();
		
		// Make sure there is a current selection
		if (nCurSel == -1) {
			return false;
		}

		// Based on the current selection, get the location id that we're dealing with
		long nLocID = VarLong(m_list->GetValue(nCurSel, 0), -1);
		
		// Make sure the location id is valid
		if (nLocID == -1) {
			return false;
		}

		// Open the recordset of the particular office hours for this location
		_RecordsetPtr rs;
		rs = CreateRecordset("SELECT %s FROM LocationsT  WHERE ID = %li", strFieldName, nLocID);

		// Get the value that's currently stored in the data
		_variant_t varStoredTime = rs->Fields->GetItem((LPCTSTR)strFieldName)->GetValue();
		if (varStoredTime.vt == VT_DATE) {
			// If there is a good time in there and they are trying 
			// to put in a different good time then we let them
			if (dtGivenTime.GetStatus() == COleDateTime::valid) {
				// The given time is valid so format it 
				// correctly and show it on screen as such
				CString str;
				SetSafeDlgText(nId, FormatDateTimeForInterface(dtGivenTime, DTF_STRIP_SECONDS, dtoTime));
				GetDlgItemText(nPartnerId, str); // We want the partner text for start/end time validation
				dtPartnerToBeSaved.ParseDateTime(str);
				//Prepare to save.
				dtToBeSaved = dtGivenTime;
			} else {
				dtToBeSaved.SetStatus(COleDateTime::invalid);
				dtPartnerToBeSaved.SetStatus(COleDateTime::invalid);
				bDoSavePartner = true;
			}

			// Make sure the start time is before the end time
			if (dtToBeSaved.GetStatus() == COleDateTime::valid &&
				dtPartnerToBeSaved.GetStatus() == COleDateTime::valid)
			{
				if (nId == IDC_OPENSUN || nId == IDC_OPENMON || nId == IDC_OPENTUES || nId == IDC_OPENWED ||
					nId == IDC_OPENTHUR || nId == IDC_OPENFRI || nId == IDC_OPENSAT)
				{
					if (dtToBeSaved >= dtPartnerToBeSaved)
					{
						dtToBeSaved.SetStatus(COleDateTime::invalid);
						dtPartnerToBeSaved.SetStatus(COleDateTime::invalid);
						bDoSavePartner = true;
					}
				}
				else
				{
					if (dtToBeSaved <= dtPartnerToBeSaved)
					{
						dtToBeSaved.SetStatus(COleDateTime::invalid);
						dtPartnerToBeSaved.SetStatus(COleDateTime::invalid);
						bDoSavePartner = true;
					}
				}
			}

		} else {
			// There is a bad date in the data 
			// If the user is trying to set one of the values to a 
			// good time, then save it and set the partner to its default
			if (dtGivenTime.GetStatus() == COleDateTime::valid) {
				dtToBeSaved = dtGivenTime;
				dtPartnerToBeSaved.ParseDateTime(GetCtrlDefaultString(nPartnerId));
				bDoSavePartner = true;
			} else {
				dtToBeSaved.SetStatus(COleDateTime::invalid);
				dtPartnerToBeSaved.SetStatus(COleDateTime::invalid);
				bDoSavePartner = true;
				//there was a bad date in the data and they are trying to enter a bad date,
				//then we don't need to do anything because CLOSED should already be
				//in the dialog box
			}
		}

		// Display that which we are about to save.
		SetDlgItemText(nId, dtToBeSaved.GetStatus() == COleDateTime::invalid ? "CLOSED" : FormatDateTimeForInterface(dtToBeSaved, DTF_STRIP_SECONDS, dtoTime));
		if(bDoSavePartner) {
			SetDlgItemText(nPartnerId, dtPartnerToBeSaved.GetStatus() == COleDateTime::invalid ? "CLOSED" : FormatDateTimeForInterface(dtPartnerToBeSaved, DTF_STRIP_SECONDS, dtoTime));
		}
		
		// Now, save it to data.
		try {
			
			ExecuteSql(
				"UPDATE LocationsT SET LocationsT.%s = %s "
				"WHERE LocationsT.ID = %li", strFieldName, 
				dtToBeSaved.GetStatus() == COleDateTime::invalid ? _T("NULL") : (_T("'") + _Q(FormatDateTimeForSql(dtToBeSaved, dtoTime)) + _T("'")), 
				nLocID);
			if(bDoSavePartner) {
				ExecuteSql(
					"UPDATE LocationsT SET LocationsT.%s = %s "
					"WHERE LocationsT.ID = %li", strPartnerName,						
					dtPartnerToBeSaved.GetStatus() == COleDateTime::invalid ? _T("NULL") : (_T("'") + _Q(FormatDateTimeForSql(dtPartnerToBeSaved, dtoTime)) + _T("'")), 
					nLocID);
			}

			// Tell everyone else we've updated some office hours
			CClient::RefreshTable(NetUtils::OfficeHours, nLocID);
		}NxCatchAllCall("Error saving OfficeHours", return false;);
		
//////	Why do we need to reload all the data?
//		Load();
		bOfficeHoursRunning = false;
		return true;
	} else {
		// Did nothing
		return false;
	}
}

CString CPracticeInfo::GetCtrlFieldName(UINT nId)
{
	switch (nId) {
	// Just take the Control ID and convert it to a data fieldname
	case IDC_OPENSUN:		return "SundayOpen";
	case IDC_OPENMON:		return "MondayOpen";
	case IDC_OPENTUES:	return "TuesdayOpen";
	case IDC_OPENWED:		return "WednesdayOpen";
	case IDC_OPENTHUR:	return "ThursdayOpen";
	case IDC_OPENFRI:		return "FridayOpen";
	case IDC_OPENSAT:		return "SaturdayOpen";
	case IDC_CLOSESUN:	return "SundayClose";
	case IDC_CLOSEMON:	return "MondayClose";
	case IDC_CLOSETUES:	return "TuesdayClose";
	case IDC_CLOSEWED:	return "WednesdayClose";
	case IDC_CLOSETHUR:	return "ThursdayClose";
	case IDC_CLOSEFRI:	return "FridayClose";
	case IDC_CLOSESAT:	return "SaturdayClose";
	default:
		// Failure
		return "";
	}
}

CString CPracticeInfo::GetCtrlDefaultString(UINT nId)
{
	switch (nId) {
	case IDC_OPENSUN: case IDC_OPENMON: case IDC_OPENTUES: case IDC_OPENWED:
	case IDC_OPENTHUR: case IDC_OPENFRI: case IDC_OPENSAT:
		// All officehours open-times default to 9am
		return FormatDateTimeForInterface(COleDateTime(1899,12,30,9,0,0), DTF_STRIP_SECONDS, dtoTime);
	case IDC_CLOSESUN: case IDC_CLOSEMON: case IDC_CLOSETUES: case IDC_CLOSEWED:
	case IDC_CLOSETHUR: case IDC_CLOSEFRI: case IDC_CLOSESAT:
		// All officehours close-times default to 5pm
		return FormatDateTimeForInterface(COleDateTime(1899,12,30,17,0,0), DTF_STRIP_SECONDS, dtoTime);
	default:
		// Failure
		return "";
	}
}

UINT CPracticeInfo::GetCtrlPartnerId(UINT nId)
{
	switch (nId) {
	// The partner of an OPEN is a CLOSE and vice versa
	case IDC_OPENSUN:		return IDC_CLOSESUN;
	case IDC_OPENMON:		return IDC_CLOSEMON;
	case IDC_OPENTUES:	return IDC_CLOSETUES;
	case IDC_OPENWED:		return IDC_CLOSEWED;
	case IDC_OPENTHUR:	return IDC_CLOSETHUR;
	case IDC_OPENFRI:		return IDC_CLOSEFRI;
	case IDC_OPENSAT:		return IDC_CLOSESAT;
	case IDC_CLOSESUN:	return IDC_OPENSUN;
	case IDC_CLOSEMON:	return IDC_OPENMON;
	case IDC_CLOSETUES:	return IDC_OPENTUES;
	case IDC_CLOSEWED:	return IDC_OPENWED;
	case IDC_CLOSETHUR:	return IDC_OPENTHUR;
	case IDC_CLOSEFRI:	return IDC_OPENFRI;
	case IDC_CLOSESAT:	return IDC_OPENSAT;
	default:
		// Failure
		return 0;
	}
}

// Returns true if the field was saved successfully
bool CPracticeInfo::PreKillFocusDlgItem(UINT nId)
{
	switch (nId) {
    // (j.gruber 2009-10-07 17:29) - PLID 35826 - lookup city
	case IDC_PRACCITY:
		return PreKillFocusCity();
	break;
	case IDC_PRACZIP:
		// Zipcode is special
		return PreKillFocusZipCode();
	case IDC_OPENSUN: case IDC_OPENMON: case IDC_OPENTUES: case IDC_OPENWED: 
	case IDC_OPENTHUR: case IDC_OPENFRI: case IDC_OPENSAT: 
	case IDC_CLOSESUN: case IDC_CLOSEMON: case IDC_CLOSETUES: case IDC_CLOSEWED: 
	case IDC_CLOSETHUR: case IDC_CLOSEFRI: case IDC_CLOSESAT:
		// All the OfficeHours fields are handled specially
		return PreKillFocusOfficeHours(nId);
		break;
	case IDC_PRACNOTES_BOX:
		return !ForceFieldLength((CNxEdit*)GetDlgItem(IDC_PRACNOTES_BOX), 255);
		break;
	default:
		// Nothing was saved yet
		m_changed = true;
		return false;
	}
}

BOOL CPracticeInfo::OnCommand(WPARAM wParam, LPARAM lParam)
{
	static CString str;

	switch (HIWORD(wParam)) {
	case EN_CHANGE:
		{
			// (d.moore 2007-04-23 12:11) - PLID 23118 - 
			//  Capitalize letters in the zip code as they are typed in. Canadian postal
			//    codes need to be formatted this way.
			int nID = LOWORD(wParam);
			if (nID == IDC_PRACZIP) {
				CapitalizeAll(IDC_PRACZIP);
			}
			//if the page is loading then skip this, else do it
			m_changed = true;
			FormatDlgItem(LOWORD(wParam));		
		}
	break;

	case EN_KILLFOCUS:
		{
			int nID = LOWORD(wParam);
			switch (nID) {
				case IDC_PRACMAINNUM:
				case IDC_PRACALTNUM:
				case IDC_PRACFAXNUM:
				case IDC_MODEM:
					if (SaveAreaCode(nID)) {
						Save(nID);
					}
				break;

				default:
				
					// Run the Pre-kill and if it doesn't save, do the default save
					if (PreKillFocusDlgItem(LOWORD(wParam))) {
					m_changed = false;
					}
					if (m_changed) {
						Save(LOWORD(wParam));
					}
				break;
			}
		}
	break;

	case EN_SETFOCUS:
		{
			int nID = LOWORD(wParam);
			switch (nID) {
				case IDC_PRACMAINNUM:
				case IDC_PRACALTNUM:
				case IDC_PRACFAXNUM:
				case IDC_MODEM:
					if (ShowAreaCode()) {
						FillAreaCode(nID);
					}
				break;
				default:
				break;
			}
		}
	break; 

	}
	return CNxDialog::OnCommand(wParam, lParam);
}

void CPracticeInfo::OnAddlocation() 
{
	try {
		// (a.walling 2009-03-31 14:38) - PLID 33573 - Support adding location from pharmacy directory
		// since I'll be doing a popup box here, might as well support all different types.
		CMenu mnu;
		if (!mnu.CreatePopupMenu())
			return;

		enum EMenuItems {
			miGeneral = 1,
			miLab,
			miPharmacy,
			miPharmacyFromDirectory,
		};

		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION|MF_DEFAULT, miGeneral, "&General Location");
		mnu.AppendMenu(MF_SEPARATOR);
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miLab, "&Lab");
		mnu.AppendMenu(MF_ENABLED|MF_STRING|MF_BYPOSITION, miPharmacy, "&Pharmacy");

		mnu.SetDefaultItem(miGeneral, FALSE);
		
		if (g_pLicense->CheckForLicense(CLicense::lcePrescribe, CLicense::cflrSilent)) {
			mnu.AppendMenu(MF_BYPOSITION, miPharmacyFromDirectory, "Pharmacy From &Directory");
		}

		CPoint pt;
		GetMessagePos(pt);
		int nRet = mnu.TrackPopupMenu(TPM_LEFTALIGN|TPM_RETURNCMD, pt.x, pt.y, this);

		CString strLocationType;
		long nLocationType = 1;

		switch (nRet) {
			case miGeneral:
				strLocationType = "location";
				nLocationType = 1;
				break;
			case miLab:
				strLocationType = "lab";
				nLocationType = 2;
				break;
			case miPharmacy:
				strLocationType = "pharmacy";
				nLocationType = 3;
				break;
			case miPharmacyFromDirectory:
				{
					CPharmacyDirectorySearchDlg dlg(this);
					dlg.m_bMultiMode = TRUE;
					if (IDOK == dlg.DoModal()) { // something was added
						m_list->Requery(); // and requery the list
						if (dlg.m_nSelectedID != -1) {
							m_list->WaitForRequery(dlPatienceLevelWaitIndefinitely);
							m_list->SetSelByColumn(0, dlg.m_nSelectedID);
						}
						Load();
					}

					return;
				}
				break;

			default:
				ASSERT(FALSE);
			case 0:
				return;
		}

		CString strNewName, tmpStr;
		long nNewNumber;
		//give them a InputBox and have them input a new name for the datalist
		if (IDOK == InputBoxLimited(this, FormatString("Please enter a name for the new %s", strLocationType), strNewName, "",255,false,false,NULL)) {

			try {
				// CAH 7/8/01 /////////////////////////
				_RecordsetPtr rs;
				rs = CreateRecordset("SELECT Name FROM LocationsT WHERE Name = '%s'", _Q(strNewName));
				if(!rs->eof)
				{
					MessageBox("There is already a location with this name. Please choose another name.","Practice",MB_OK|MB_ICONEXCLAMATION);
					rs->Close();
					return;
				}
				rs->Close();
				///////////////////////////////////////

				//get the next consectutive id from LocationsT
				nNewNumber = NewNumber("LocationsT", "ID");
				//insert the new id into the datalist		
				CSqlTransaction sqlTrans;
				sqlTrans.Begin();
				ExecuteParamSql("INSERT INTO LocationsT (ID, Name, TypeID) VALUES ({INT}, {STRING}, {INT})", nNewNumber, strNewName, nLocationType);
				//TES 8/4/2011 - PLID 44862 - If this is a General-type location, add a user-defined permission object.
				if(nLocationType == 1) {
					LPCTSTR strDescription = "Controls access to view or create Labs for this location.";
					long nNewSecurityObjectID = AddUserDefinedPermissionToData(nNewNumber, sptView, strNewName, strDescription, bioLabLocations, sptView);
					AddUserDefinedPermissionToMemory(nNewSecurityObjectID, nNewNumber, sptView, strNewName, strDescription, bioLabLocations, sptView);
				}
				// (j.jones 2010-06-25 11:53) - PLID 39185 - link with all labs to be ordered records
				if(nLocationType == 2) {
					ExecuteParamSql("INSERT INTO LabsToBeOrderedLocationLinkT (LocationID, LabsToBeOrderedID) "
						"SELECT {INT}, ID FROM LabsToBeOrderedT", nNewNumber);
				}
				sqlTrans.Commit();
				
				// Network code
				// (a.walling 2007-08-06 12:36) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
				CClient::RefreshTable(NetUtils::LocationsT, nNewNumber);

			}NxCatchAll("Error in OnAddlocation()");

			//requery the location datalist to reflect the added Location
			IRowSettingsPtr   pRow;
			pRow = m_list->GetRow(-1);
			_variant_t varDlgText;
			varDlgText = strNewName;
			pRow->PutValue(0, nNewNumber);
			pRow->PutValue(1, varDlgText);
			if (nLocationType == 1) {
				pRow->PutValue(2, _variant_t("General"));
			} 
			else if (nLocationType == 2) {
				pRow->PutValue(2, _variant_t("Lab"));
				// (z.manning 2010-02-10 17:22) - PLID 24044 - We are adding a new lab so add it to the 
				// list of default labs.
				NXDATALIST2Lib::IRowSettingsPtr pDefaultLabRow = m_pdlDefaultLab->GetNewRow();
				pDefaultLabRow->PutValue(dlccID, nNewNumber);
				pDefaultLabRow->PutValue(dlccLabName, varDlgText);
				m_pdlDefaultLab->AddRowSorted(pDefaultLabRow, NULL);
			} 
			else if (nLocationType == 3) {
				pRow->PutValue(2, _variant_t("Pharmacy"));
			} 
			else { 
				ASSERT(FALSE);
				pRow->PutValue(2, g_cvarNull);
			}
			m_list->AddRow(pRow);
			//set the location datalist's current selection to be the newly added location
			m_list->SetSelByColumn(0, nNewNumber);

			//auditing
			long nAuditID = -1;
			nAuditID = BeginNewAuditEvent();
			if(nAuditID != -1)
				AuditEvent(-1, "", nAuditID, aeiLocationCreate, nNewNumber, "", strNewName, aepMedium, aetCreated);
			//requery the dialog screen to the new location's information
			Load();
		}//end if IDOK
	} NxCatchAll("CPracticeInfo::OnAddlocation");
}

void CPracticeInfo::OnDeletelocation() 
{
	// m.carlson 2/9/2004 PLID 10901
	if (m_list->GetRowCount() == 1)
	{
		AfxMessageBox("Practice requires at least 1 location.");
		return;
	}

	if (m_list->CurSel == -1)
		return;


	//make sure that they absolutely want to delete this location
	if (IDYES != MsgBox(MB_YESNO,
		"This will DELETE this location permanently. Are you absolutely sure you want to continue?"))
	{
		return;
	}
	
	try 
	{
		_RecordsetPtr rs;
		_variant_t varCurSelVal = m_list->Value[m_list->CurSel][0];

		//JMJ 3/10/03 - by replicating code from OnRemove(), these checks ensure that in no way can you configure the system where nobody can log in

		//first check that there will be at least one managed location
		if(IsRecordsetEmpty("SELECT ID FROM LocationsT WHERE Managed = 1 AND ID != %li",
			VarLong(m_list->Value[m_list->CurSel][0])) && !m_managed.GetCheck()) {
			AfxMessageBox("This is the only managed location.\n"
				"Disabling this will mean that nobody can log in to Practice from any location.\n"
				"The change will not be made.",MB_ICONEXCLAMATION);

			m_managed.SetCheck(TRUE);
			return;
		}

		//now check to make sure that there are not going to be any locations that users can't log in to
		//NOTE: technically this query could/should replace the above check, however I (JMJ) feel that the above check
		//is faster and can give a more accurate answer to the user as to what the heck is going on. 
		//Keep in mind that unchecking the last managed location is more common than the following problem:
		if(IsRecordsetEmpty("SELECT PersonID FROM UserLocationT INNER JOIN LocationsT ON UserLocationT.LocationID = LocationsT.ID WHERE Managed = 1 AND LocationID != %li",
			VarLong(m_list->Value[m_list->CurSel][0]))) {

			AfxMessageBox("This operation will disallow any users to log in to Practice.\n"
				"You must have at least one user with the ability to log in.\n\n"
				"If you still wish to delete this location, select another managed location \n"
				"and allow users to log in to it.");

			return;
		}

		// (e.lally 2005-5-17 16:00) - User cannot delete the location they are logged in at
			//otherwise the Inventory module would cause an error
		long nLocToDelete;
		nLocToDelete = VarLong(varCurSelVal);
		if(GetCurrentLocation() == nLocToDelete)
		{
			MsgBox(MB_ICONWARNING|MB_OK,"You cannot be logged into the location you wish to delete.");
			return;
		}
		// (e.lally 2005-5-17 16:00) - Do not allow location to be deleted if Persons are
		//		still set to that location.
		_RecordsetPtr prsPersons = CreateRecordset("SELECT top 1 ID "
			"FROM PersonT WHERE location= %li", nLocToDelete);
		if(!prsPersons->eof)
		{
			MsgBox(MB_ICONWARNING|MB_OK,"This location has patients or contacts associated with it.\n"
					"Please reassign associated patients to a different location.");
			return;
		}

		//DRT 3/16/2006 - PLID 19749 - You are not allowed to delete a location tied to a prescription.
		if(ReturnsRecords("SELECT TOP 1 PharmacyID FROM PatientMedications WHERE PharmacyID = %li", nLocToDelete)) {
			MsgBox(MB_ICONWARNING|MB_OK, "You may not delete this location because it has prescriptions tied to it.  Please mark the location inactive instead.");
			return;
		}

		// (z.manning, 06/05/2007) - PLID 14238 - Don't let them delete a location that's tied to a NexPDA profile.
		// (z.manning 2009-11-12 15:35) - PLID 31879 - May also be NexSync
		if(ReturnsRecords("SELECT TOP 1 LocationID FROM OutlookCalendarT WHERE LocationID = %li", nLocToDelete)) {
			MessageBox("This location is tied to NexPDA and/or NexSync profile(s) and may not be deleted. Please mark the location inactive instead.");
			return;
		}

		// (c.haag 2007-11-13 16:30) - PLID 27994 - Don't let them delete a location that's associated with inventory
		// returns
		if (ReturnsRecords("SELECT ID FROM SupplierReturnGroupsT WHERE LocationID = %d", nLocToDelete))
		{
			MsgBox("This location is associated with one or more supplier inventory returns. Please delete these returns if you wish to remove this location.");
			return;			
		}

		// (j.gruber 2008-06-25 14:54) - PLID 26136 - not allowed to delete a location with time logs associated with it
		if (ReturnsRecords("SELECT ID FROM UserTimesT WHERE LocationID = %d", nLocToDelete))
		{
			MsgBox("This location is associated with one or more employee time log entries. Please mark the location inactive instead.");
			return;			
		}
		
		// (a.walling 2009-04-06 15:33) - PLID 33865 - Do not delete if pharmacy staff associated here
		if (ReturnsRecords("SELECT PersonID FROM PharmacyStaffT WHERE PharmacyID = %d", nLocToDelete))
		{
			MsgBox("One or more pharmacy staff are associated with this location. Please delete the staff records individually before deleting, or mark the location inactive instead.");
			return;			
		}

		//TES 6/21/2010 - PLID 5888 - Don't delete locations that are used in Resource Availability templates
		if (ReturnsRecords("SELECT LocationID FROM ResourceAvailTemplateT WHERE LocationID = %d", nLocToDelete))
		{
			//(e.lally 2010-07-15) PLID 39626 - Renamed to Location Templates
			MsgBox("This location is used on one or more Location Templates.  Please delete or re-assign these templates before deleting, or mark the location inactive instead.");
			return;
		}

		// (r.farnworth 2015-03-11 11:51) - PLID 59962 - Don't delete locations that are used as Performing Locations on a Lab Result.
		if (ReturnsRecords("SELECT PerformingLabID FROM LabResultsT WHERE PerformingLabID = %d", nLocToDelete))
		{
			MsgBox("This location is used on one or more Lab Results as a Performing Lab.  Please mark the location inactive instead.");
			return;
		}

		// (s.tullis 2014-12-12 12:07) - PLID 64440
		rs = CreateParamRecordset("Select LocationID FROM ScheduleMixRuleLocationsT  WHERE LocationID = {INT}", nLocToDelete);
		if (!rs->eof)
		{
			MsgBox("This location is used on one or more Scheduling Mix Rule Templates.  Please delete or re-assign these templates before deleting, or mark the location inactive instead.");
			return;
		}
		// (s.dhole 2010-11-19 10:00) - PLID 41552 Check if location used in VsionWeb than don not allow user to delete this record
		// (s.dhole 2010-03-10 10:18) - PLID 42723 change Db Table and column name changes from VisionWeb To Glasses
		rs = CreateParamRecordset("Select * from GlassesSupplierLocationsT WHERE LocationID= {INT}", nLocToDelete);
		if(!rs->eof){
			MsgBox(MB_ICONSTOP|MB_OK,"This location currently has VisionWeb data associated with it. You may not delete this location; Please mark this location as inactive.");
			return;			
		}
		else
			rs->Close(); 

		//for auditing
		long nDelID = VarLong(m_list->GetValue(m_list->CurSel, 0));
		CString strDel = CString(m_list->GetValue(m_list->CurSel, 1).bstrVal);

		rs = CreateRecordset("SELECT Count(ID) "
			"FROM BillsT "
			"WHERE Location = %li", 
			varCurSelVal.lVal);
		if (!(rs->eof && rs->bof || rs->Fields->Item[0L]->Value.lVal == 0))
		{	MsgBox(MB_ICONWARNING|MB_OK, 
				"You may not delete this record because it has a corresponding bill.\n"
				"Please delete the bill before attempting to delete this location.");
			return;
		}
		rs->Close();

		// (j.jones 2007-02-20 08:47) - PLID 24204 - ensured you cannot delete a location used on charges or payments
		if(ReturnsRecords("SELECT TOP 1 ID FROM LineItemT WHERE LocationID = %li", nLocToDelete)) {
			MsgBox(MB_ICONWARNING|MB_OK, "You may not delete this location because charges and/or payments associated with it.  Please mark the location inactive instead.");
			return;
		}

		// (j.gruber 2007-08-09 10:37) - PLID 25119 - we shouldn't need this because of the above, but 
		//make sure there aren't any cash drawers tied to this location
		if(ReturnsRecords("SELECT TOP 1 ID FROM CashDrawersT WHERE LocationID = %li", nLocToDelete)) {
			MsgBox(MB_ICONWARNING|MB_OK, "You may not delete this location because it has cash drawer(s) associated with it.  Please mark the location inactive instead.");
			return;
		}

		// (j.gruber 2007-02-26 09:19) - 24916 - ensure you cannot delete a location used on batch payments
		if(ReturnsRecords("SELECT TOP 1 ID FROM BatchPaymentsT WHERE Location = %li", nLocToDelete)) {
			MsgBox(MB_ICONWARNING|MB_OK, "You may not delete this location because it has batch payments associated with it.  Please mark the location inactive instead.");
			return;
		}

		if (ReturnsRecords("SELECT ID FROM ProductLocationTransfersT WHERE SourceLocationID = %li OR DestLocationID = %li",varCurSelVal.lVal,varCurSelVal.lVal))
		{	MsgBox(MB_ICONWARNING|MB_OK, 
				"You may not delete this record because it has been involved in an inventory transfer.");
			return;
		}

		//if there are no orders or product adjustments then delete from product location infoT

		if (ReturnsRecords("SELECT ID FROM ProductAdjustmentsT WHERE LocationID = %li", VarLong(varCurSelVal))) {
			MsgBox(MB_ICONWARNING|MB_OK, 
				"You may not delete this record because it has a product adjustment associated with it in inventory");
			return;
		}

		// (j.gruber 2008-03-03 16:57) - PLID 28955 - include the sold to location as well
		if (ReturnsRecords("SELECT ID FROM OrderT WHERE LocationID = %li OR LocationSoldTo = %li", VarLong(varCurSelVal), VarLong(varCurSelVal))) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not delete this record because it has an order associated with it in inventory");
			return;
		}

		// (j.jones 2007-11-07 12:22) - PLID 27987 - disallow deleting locations with inventory allocations
		if (ReturnsRecords("SELECT ID FROM PatientInvAllocationsT WHERE LocationID = %li", VarLong(varCurSelVal))) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not delete this record because it has an inventory patient allocation associated with it.");
			return;
		}
		
		if(!IsRecordsetEmpty("SELECT ID FROM AppointmentsT WHERE LocationID = %li",varCurSelVal.lVal)) {
			MsgBox(MB_ICONWARNING|MB_OK, 
				"You may not delete this record because it has a corresponding appointment in the scheduler.\n"
				"Please delete the appointment before attempting to delete this location.");
			return;
		}

		if(!IsRecordsetEmpty("SELECT ID FROM EMRMasterT WHERE LocationID = %li", varCurSelVal.lVal)){
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not delete this record because it has at least one corresponding EMR.\n"
				"Please change the location of the EMR before attempting to delete this location.");
			return;
		}

		//DRT 6/30/2006 - PLID 21291 - Do not allow deleting locations tied to labs (as either the lab or the practice location)
		if(ReturnsRecords("SELECT ID FROM LabsT WHERE LocationID = %li OR LabLocationID = %li", VarLong(varCurSelVal), VarLong(varCurSelVal))) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not delete a location that is tied to a lab entry.  Please inactivate this location instead.");
			return;
		}

		// (j.jones 2007-05-23 15:01) - PLID 8993 - disallow if in an Eligibility Request
		if (ReturnsRecords("SELECT ID FROM EligibilityRequestsT WHERE LocationID = %li", VarLong(varCurSelVal))) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not delete this record because it is selected in at least one E-Eligibility Request.");
			return;
		}

		// (j.jones 2009-01-13 17:29) - PLID 26141 - disallow if linked to an inv. reconciliation
		if (ReturnsRecords("SELECT ID FROM InvReconciliationsT WHERE LocationID = %li", VarLong(varCurSelVal))) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not delete this record because it is linked to at least one Inventory Reconciliation.");
			return;
		}

		// (j.jones 2009-08-03 16:17) - PLID 24600 - disallow if linked to a room manager room
		if (ReturnsRecords("SELECT ID FROM RoomsT WHERE LocationID = %li", VarLong(varCurSelVal))) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not delete this record because it is linked to at least one room in Room Manager.");
			return;
		}

		// (j.gruber 2011-06-17 12:17) - PLID 43957 - GlassesOrderT.LocationID
		if (ReturnsRecords("SELECT ID FROM GlassesOrderT WHERE LocationID = %li", VarLong(varCurSelVal))) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not delete this record because it is linked to at least one Glasses or Contact Lens order. Please mark the location inactive instead.");
			return;
		}

		// (j.jones 2011-10-06 16:43) - PLID 45828 - if someone is logged in to this location from a device, you cannot delete it
		if(ReturnsRecordsParam("SELECT TOP 1 LocationID FROM UserLoginTokensT WHERE LocationID = {INT}", VarLong(varCurSelVal))) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"At least one user is currently logged in to this location from a device. The location cannot be deleted while a user is logged in.");
			return;
		}

		// (b.savon 2013-06-13 10:41) - PLID 56867 - NexERxPrescriberRegistrationT.LocationID
		if(ReturnsRecordsParam("SELECT TOP 1 LocationID FROM NexERxPrescriberRegistrationT WHERE LocationID = {INT}", VarLong(varCurSelVal))) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"At least one provider is currently registered for NexERx at this location.  Please mark the location inactive instead.");
			return;
		}

		// (j.jones 2016-02-17 17:18) - PLID 68348 - check recalls
		if (ReturnsRecordsParam("SELECT TOP 1 LocationID FROM RecallT WHERE LocationID = {INT}", VarLong(varCurSelVal))) {
			MsgBox(MB_ICONWARNING | MB_OK,
				"You may not delete this location because it is linked to at least one recall.  Please mark the location inactive instead.");
			return;
		}

		CString strWarning = "The following resources are linked with this location:\r\n\r\n";
		BOOL bShowWarning = FALSE;
		_RecordsetPtr prsResources = CreateRecordset("SELECT Item FROM ResourceLocationConnectT LEFT JOIN ResourceT ON ResourceLocationConnectT.ResourceID = ResourceT.ID WHERE LocationID = %d",
			varCurSelVal.lVal);
		while (!prsResources->eof)
		{
			if (!bShowWarning)
				bShowWarning = TRUE;
			CString str;
			str.Format("%s\r\n", AdoFldString(prsResources, "Item", ""));
			strWarning += str;
			prsResources->MoveNext();
		}
		if (bShowWarning)
		{
			strWarning += "\r\nDeleting this location will result in no location being linked with these resources. Do you still wish to continue?";
			if (IDYES != MsgBox(MB_YESNO, strWarning))
			{
				return;
			}
		}
		prsResources->Close();
		prsResources.Release();

		// (c.haag 2004-12-28 13:08) - Ask about multi-fee information
		// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee group to fee schedule
		strWarning = "The following fee schedules are linked with this location:\r\n\r\n";
		prsResources = CreateRecordset("SELECT Name FROM MultiFeeLocationsT LEFT JOIN MultiFeeGroupsT ON MultiFeeLocationsT.FeeGroupID = MultiFeeGroupsT.ID WHERE LocationID = %d",
			varCurSelVal.lVal);
		bShowWarning = FALSE;
		while (!prsResources->eof)
		{
			if (!bShowWarning)
				bShowWarning = TRUE;
			CString str;
			str.Format("%s\r\n", AdoFldString(prsResources, "Name", ""));
			strWarning += str;
			prsResources->MoveNext();
		}
		if (bShowWarning)
		{
			strWarning += "\r\nDo you still wish to delete this location?";
			if (IDYES != MsgBox(MB_YESNO, strWarning))
			{
				return;
			}
		}

		// (b.cardillo 2011-02-26 11:01) - These should have been closed or just put in their own code blocks, but weren't.
		prsResources->Close();
		prsPersons->Close();

		// (b.cardillo 2011-02-26 10:15) - PLID 40419 - Check for appointment prototypes that reference this location
		{
			// Get the list of prototypes, if any, that depend on this location
			CString strReferencedPrototypes = GenerateDelimitedListFromRecordsetColumn(CreateParamRecordset(
					_T("SELECT Name FROM ApptPrototypeT WHERE ID IN (\r\n")
					_T(" SELECT PS.ApptPrototypeID \r\n")
					_T(" FROM ApptPrototypePropertySetT PS \r\n")
					_T(" INNER JOIN ApptPrototypePropertySetLocationT PSL ON PS.ID = PSL.ApptPrototypePropertySetID \r\n")
					_T(" WHERE PSL.LocationID = {INT} \r\n")
					_T(") \r\n")
					_T("ORDER BY Name \r\n")
					, VarLong(varCurSelVal))
				, AsVariant("Name"), "", "\r\n");
			if (!strReferencedPrototypes.IsEmpty()) {
				MsgBox(MB_ICONWARNING|MB_OK, 
					_T("You may not delete this location because it is referenced by the following Appointment Prototypes:\r\n\r\n%s")
					, strReferencedPrototypes);
				return;
			}
		}

		// (s.tullis 2016-05-25 15:38) - NX-100760 -  EyeProceduresT.LocationID is not nullable do not allow them to delete
		if (ReturnsRecordsParam("Select TOP 1 ID FROM EyeProceduresT Where LocationID = {INT} ", VarLong(varCurSelVal)))
		{
			MsgBox(MB_ICONWARNING | MB_OK, "You may not delete this record because it is linked to at least one Eye Procedure. Please mark the location inactive instead.");
			return;
		}
		
		CString strSql = BeginSqlBatch();

		//TES 9/8/2008 - PLID 27727 - Dropped DefLocationID
		//AddStatementToSqlBatch(strSql, "UPDATE ProvidersT SET DefLocationID = (SELECT TOP 1 ID FROM LocationsT WHERE Active = 1 AND Managed = 1 AND ID <> %li) WHERE DefLocationID = %li",varCurSelVal.lVal, varCurSelVal.lVal);
		// (s.tullis 2016-05-25 15:38) - NX-100760 -  Update and delete these tables
		AddStatementToSqlBatch(strSql, "Update MailSent SET Location = NULL WHERE location = %li ", varCurSelVal.lVal);

		AddStatementToSqlBatch(strSql, "Update TodoList SET LocationID = NULL WHERE locationID = %li ", varCurSelVal.lVal);

		AddStatementToSqlBatch(strSql, "Update ResourceViewsT SET LocationID = NULL WHERE locationID = %li ", varCurSelVal.lVal);

		AddStatementToSqlBatch(strSql, "Update PersonT SET Location = NULL WHERE location = %li ", varCurSelVal.lVal);

		AddStatementToSqlBatch(strSql, "DELETE FROM LocationCustomReportsT WHERE LocationID = %li", varCurSelVal.lVal);
		//make sure that ProductLocationInfoT is cleared
		AddStatementToSqlBatch(strSql, "DELETE FROM ProductLocationInfoT WHERE LocationID = %li",varCurSelVal.lVal);

		// (j.gruber 2012-12-04 08:51) - PLID 48566 - make sure ServiceLocationInfoT is Cleared
		AddStatementToSqlBatch(strSql, "DELETE FROM ServiceLocationInfoT WHERE LocationID = %li",varCurSelVal.lVal);

		//clear out the Product Responsibility table
		AddStatementToSqlBatch(strSql, "DELETE FROM ProductResponsibilityT WHERE LocationID = %li", VarLong(varCurSelVal));

		//clear out of medications
		AddStatementToSqlBatch(strSql, "UPDATE PatientMedications SET LocationID = NULL WHERE LocationID = %li", varCurSelVal.lVal);

		AddStatementToSqlBatch(strSql, "UPDATE InsuranceReferralsT SET LocationID = NULL WHERE LocationID = %li", varCurSelVal.lVal);
		
		AddStatementToSqlBatch(strSql, "UPDATE MarketingCostsT SET LocationID = NULL WHERE LocationID = %li", varCurSelVal.lVal);

		//remove access to login from here
		AddStatementToSqlBatch(strSql, "DELETE FROM UserLocationT WHERE UserLocationT.LocationID = %li", varCurSelVal.lVal);

		//delete the insurance IDs
		AddStatementToSqlBatch(strSql, "DELETE FROM InsuranceFacilityID WHERE LocationID = %li", varCurSelVal.lVal);
		AddStatementToSqlBatch(strSql, "DELETE FROM AdvHCFAPinT WHERE LocationID = %li", varCurSelVal.lVal);
		// (j.jones 2007-01-29 14:35) - PLID 24411 - remove from EbillingSetup tables
		AddStatementToSqlBatch(strSql, "DELETE FROM EbillingSetupT WHERE LocationID = %li", varCurSelVal.lVal);
		AddStatementToSqlBatch(strSql, "DELETE FROM UB92EbillingSetupT WHERE LocationID = %li", varCurSelVal.lVal);
		// (j.jones 2008-06-23 10:21) - PLID 30434 - added EligibilitySetupT
		AddStatementToSqlBatch(strSql, "DELETE FROM EligibilitySetupT WHERE LocationID = %li", varCurSelVal.lVal);

		AddStatementToSqlBatch(strSql, "DELETE FROM POSLocationLinkT WHERE LocationID = %li", varCurSelVal.lVal);
		AddStatementToSqlBatch(strSql, "DELETE FROM ResourceLocationConnectT WHERE LocationID = %li", varCurSelVal.lVal);
		// (c.haag 2003-07-30 15:49) - The CurResourceView property number is a location ID
		AddStatementToSqlBatch(strSql, "DELETE FROM ConfigRT WHERE NAME = 'CurResourceView' AND Number = %li", varCurSelVal.lVal);
		AddStatementToSqlBatch(strSql, "DELETE FROM ConfigBillColumnsT WHERE LocationID = %li",varCurSelVal.lVal);
		// (c.haag 2004-12-28 13:01) - Delete multi-fee information
		AddStatementToSqlBatch(strSql, "DELETE FROM MultiFeeLocationsT WHERE LocationID = %li",varCurSelVal.lVal);
		// (j.jones 2007-10-15 14:49) - PLID 27757 - handle deleting anesth/facility setups
		AddStatementToSqlBatch(strSql, "DELETE FROM LocationFacilityFeesT WHERE FacilityFeeSetupID IN (SELECT ID FROM FacilityFeeSetupT WHERE LocationID = %li)",varCurSelVal.lVal);
		AddStatementToSqlBatch(strSql, "DELETE FROM FacilityFeeSetupT WHERE LocationID = %li",varCurSelVal.lVal);		
		AddStatementToSqlBatch(strSql, "DELETE FROM LocationAnesthesiaFeesT WHERE AnesthesiaSetupID IN (SELECT ID FROM AnesthesiaSetupT WHERE LocationID = %li)",varCurSelVal.lVal);
		AddStatementToSqlBatch(strSql, "DELETE FROM AnesthesiaSetupT WHERE LocationID = %li",varCurSelVal.lVal);
		//TES 11/2/2006 - PLID 23314 - Remove any HL7 mappings.
		//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT
		AddStatementToSqlBatch(strSql, "DELETE FROM HL7CodeLinkT WHERE Type = %i AND PracticeID = %li", hclrtLocation, varCurSelVal.lVal);

		// (r.gonet 01/15/2013) - PLID 54287 - HL7 Facility Code Overrides now filter on location.
		AddStatementToSqlBatch(strSql, "DELETE FROM HL7OverrideFacilityCodesT WHERE LocationID = %li", varCurSelVal.lVal);
		
		// (j.jones 2008-10-07 17:16) - PLID 31596 - remove favorite pharmacies
		RemoveFavoritePharmacy(strSql, VarLong(varCurSelVal));

		// (j.jones 2009-02-09 09:40) - PLID 32951 - remove CLIA setup
		AddStatementToSqlBatch(strSql, "DELETE FROM CLIANumbersT WHERE LocationID = %li", VarLong(varCurSelVal));

		// (j.jones 2010-07-06 15:21) - PLID 39506 - delete from ClearinghouseLoginInfoT
		AddStatementToSqlBatch(strSql, "DELETE FROM ClearinghouseLoginInfoT WHERE LocationID = %li", VarLong(varCurSelVal));

		// (a.walling 2009-03-31 11:25) - PLID 33758 - Delete from PharmacyID table
		AddStatementToSqlBatch(strSql, "DELETE FROM PharmacyIDT WHERE LocationID = %li", varCurSelVal.lVal);

		// (j.jones 2009-08-05 08:42) - PLID 34467 - delete from InsuranceLocationPayerIDsT
		AddStatementToSqlBatch(strSql, "DELETE FROM InsuranceLocationPayerIDsT WHERE LocationID = %li", varCurSelVal.lVal);

		// (z.manning 2010-01-11 12:17) - PLID 24044 - Handle DefaultLabID
		AddStatementToSqlBatch(strSql, "UPDATE LocationsT SET DefaultLabID = NULL WHERE DefaultLabID = %li", varCurSelVal.lVal);

		//TES 2/1/2010 - PLID 37143 - Need to clear out any per-location default custom reports
		AddStatementToSqlBatch(strSql, "DELETE FROM LocationCustomReportsT WHERE LocationID = %li", VarLong(varCurSelVal));

		// (j.jones 2010-06-25 12:01) - PLID 39185 - clear out LabsToBeOrderedLocationLinkT
		AddStatementToSqlBatch(strSql, "DELETE FROM LabsToBeOrderedLocationLinkT WHERE LocationID = %li", VarLong(varCurSelVal));

		//TES 7/26/2010 - PLID 39445 - Make sure this location isn't the default for any resource views.
		AddStatementToSqlBatch(strSql, "UPDATE ResourceViewsT SET LocationID = NULL WHERE LocationID = %li", VarLong(varCurSelVal));

		// (z.manning 2011-05-18 10:09) - PLID 43756
		AddStatementToSqlBatch(strSql, "UPDATE EmnTabChartsT SET LocationID = NULL WHERE LocationID = %li", VarLong(varCurSelVal));
		
		// (r.gonet 09/03/2013) - PLID 56007 - Location can be associated with med history requests
		// (r.gonet 09/01/2013) - PLID 56007 - Delete Medication History
		AddStatementToSqlBatch(strSql, 
			"DELETE FROM MedicationHistoryResponseT WHERE ID IN "
			"( "
				"SELECT MedicationHistoryResponseT.ID "
				"FROM MedicationHistoryResponseT "
				"INNER JOIN RxHistoryRequestT ON MedicationHistoryResponseT.RxHistoryRequestID = RxHistoryRequestT.ID "
				"INNER JOIN RxHistoryMasterRequestT ON RxHistoryRequestT.RxHistoryMasterRequestID = RxHistoryMasterRequestT.ID "
				"WHERE RxHistoryMasterRequestT.LocationID = %li "
			") ", VarLong(varCurSelVal));
		AddStatementToSqlBatch(strSql, 
			"DELETE FROM RxHistoryRequestT WHERE ID IN "
			"( "
				"SELECT RxHistoryRequestT.ID "
				"FROM RxHistoryRequestT "
				"INNER JOIN RxHistoryMasterRequestT ON RxHistoryRequestT.RxHistoryMasterRequestID = RxHistoryMasterRequestT.ID "
				"WHERE RxHistoryMasterRequestT.LocationID = %li "
			") ", VarLong(varCurSelVal));
		AddStatementToSqlBatch(strSql, 
			"DELETE FROM RxHistoryMasterRequestT WHERE ID IN "
			"( "
				"SELECT RxHistoryMasterRequestT.ID "
				"FROM RxHistoryMasterRequestT "
				"WHERE RxHistoryMasterRequestT.LocationID = %li "
			") ", VarLong(varCurSelVal));

		// (a.wilson 2014-5-5) PLID 61831 - clear out ChargeLevelProviderConfigT where location was used.
		AddStatementToSqlBatch(strSql, "DELETE FROM ChargeLevelProviderConfigT WHERE LocationID = %li", VarLong(varCurSelVal));

		// (z.manning 2014-08-22 09:11) - PLID 63251 - Handle UserLocationResourceExclusionT
		AddStatementToSqlBatch(strSql, "DELETE FROM UserLocationResourceExclusionT WHERE LocationID = %li", VarLong(varCurSelVal));

		// (s.tullis 2016-02-19 16:21) - PLID 68318
		AddStatementToSqlBatch(strSql, "DELETE FROM ClaimFormLocationInsuranceSetupT WHERE LocationID = %li", VarLong(varCurSelVal));

		//try to delete the item
		AddStatementToSqlBatch(strSql, "DELETE FROM LocationsT WHERE LocationsT.ID = %li", varCurSelVal.lVal);

		CSqlTransaction sqlTran;
		sqlTran.Begin();
		ExecuteSqlBatch(strSql);

		//TES 8/4/2011 - PLID 44862 - If this used to be a General-type location, remove its user-defined permission object.
		NXDATALIST2Lib::IRowSettingsPtr pTypeRow = m_pTypeList->CurSel;
		long nType = VarLong(pTypeRow->GetValue(0));
		if(nType == 1) {
			long nSecurityObjectID = DeleteUserDefinedPermissionFromData(bioLabLocations, varCurSelVal.lVal);
			DeleteUserDefinedPermissionFromMemory(nSecurityObjectID);
		}
		sqlTran.Commit();

		m_list->RemoveRow(m_list->CurSel);
		m_list->CurSel = 0;

		// (z.manning 2010-01-11 12:25) - PLID 24044 - Remove this row from the default lab combo
		NXDATALIST2Lib::IRowSettingsPtr pLabRow = m_pdlDefaultLab->FindByColumn(dlccID, varCurSelVal.lVal, NULL, VARIANT_FALSE);
		if(pLabRow != NULL) {
			m_pdlDefaultLab->RemoveRow(pLabRow);
		}

		//auditing
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, "", nAuditID, aeiLocationDelete, nDelID, strDel, "", aepMedium, aetDeleted);

		// (v.maida 2016-01-22 10:32) - PLID 68033 - Check if the deleted location is set to a default FFA 
		// location preference and, if so, reset the preference to be the default preference value.
		if (UsersHaveLocationAsDefaultFFALocationPref(varCurSelVal.lVal))
		{
			RevertDefaultFFALocationPrefForLocation(varCurSelVal.lVal);
		}

		// (a.walling 2007-08-06 12:36) - PLID 
		CClient::RefreshTable(NetUtils::LocationsT, varCurSelVal.lVal);
		Load();
	} NxCatchAll("Could not delete location");
}

void CPracticeInfo::OnManaged() 
{
	CString tmpStr;
	_variant_t  varCurSelVal;

	if(m_list->GetCurSel() == -1)
		return;
	
	//get the id of the currently selected location
	varCurSelVal = m_list->GetValue(m_list->GetCurSel(), 0);

	//check to see that the value returned from the datalist was a long because if it isn't,
	//then we can't use it so give an error
	if (varCurSelVal.vt != VT_I4) 
		return;

	try {

		long nCurLocationID = VarLong(m_list->GetValue(m_list->CurSel, 0));

		if(GetCurrentLocation() == VarLong(m_list->GetValue(m_list->CurSel, 0)))
		{
			MsgBox(MB_ICONWARNING|MB_OK,"You cannot change the 'Managed' status for a location you are logged into.");
			if (m_managed.GetCheck())
				m_managed.SetCheck(FALSE);
			else
				m_managed.SetCheck(TRUE);
			return;
		}

		//for auditing
		long nDelID = nCurLocationID;
		CString strDel = CString(m_list->GetValue(m_list->CurSel, 1).bstrVal);

		if (IDYES != MsgBox(MB_YESNO,
					"Managed location are locations users can log in from. Are you really sure you want to change this?"))
		{
			if (m_managed.GetCheck())
				m_managed.SetCheck(FALSE);
			else m_managed.SetCheck(TRUE);

			Load();
			return;		
		}

		//JMJ 3/10/03 - by replicating code from OnRemove(), these checks ensure that in no way can you configure the system where nobody can log in

		//first check that there will be at least one managed location
		if(IsRecordsetEmpty("SELECT ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND ID != %li",
			VarLong(m_list->Value[m_list->CurSel][0])) && !m_managed.GetCheck()) {
			AfxMessageBox("This is the only managed location.\n"
				"Disabling this will mean that nobody can log in to Practice from any location.\n"
				"The change will not be made.",MB_ICONEXCLAMATION);

			m_managed.SetCheck(TRUE);
			return;
		}

		//now check to make sure that there are not going to be any locations that users can't log in to
		//NOTE: technically this query could/should replace the above check, however I (JMJ) feel that the above check
		//is faster and can give a more accurate answer to the user as to what the heck is going on. 
		//Keep in mind that unchecking the last managed location is more common than the following problem:
		if(IsRecordsetEmpty("SELECT PersonID FROM UserLocationT INNER JOIN LocationsT ON UserLocationT.LocationID = LocationsT.ID WHERE Managed = 1 AND Active = 1 AND LocationID != %li",
			VarLong(m_list->Value[m_list->CurSel][0]))) {

			AfxMessageBox("This operation will disallow any users to log in to Practice.\n"
				"You must have at least one user with the ability to log in.\n\n"
				"If you still wish to change this location to not be managed, select another managed location \n"
				"and allow users to log in to it.");

			m_managed.SetCheck(TRUE);
			return;
		}

		//PLID 21089 - make sure this isn't a lab type
		_RecordsetPtr rsLab = CreateRecordset("SELECT TypeID FROM LocationsT WHERE ID = %li", VarLong(varCurSelVal));
		if (!rsLab->eof) {
			long nTypeID = AdoFldLong(rsLab, "TypeID");
			if (nTypeID == 2) {
				MsgBox("You may not change the managed status of a lab location.");
				if (m_managed.GetCheck()) {
					m_managed.SetCheck(FALSE);
				}
				else {
					m_managed.SetCheck(TRUE);
				}
				return;
			}
			else if (nTypeID == 3) {
				MsgBox("You may not change the managed status of a pharmacy location.");
				if (m_managed.GetCheck()) {
					m_managed.SetCheck(FALSE);
				}
				else {
					m_managed.SetCheck(TRUE);
				}
				return;
			}
		}

		// (d.singleton 2013-06-18 15:52) - PLID 48012 - When unchecking a location as managed and then rechecking it as managed
		// it changes the inventory items to be billable and trackable out of that location when it was set to not trackable
		// need to warn the user so they know this will happen
		if(!m_managed.GetCheck()) {
			if(MessageBox("Disabling the managed status of this location will clear out all inventory data and service code shop fees specific to this location. "
				"This action is unrecoverable. Are you sure you wish to continue?", "NexTech Practice", MB_ICONWARNING|MB_YESNO) == IDNO) {
					m_managed.SetCheck(TRUE);
					return;
			}
		}
		
		ExecuteSql("Update LocationsT SET LocationsT.Managed = %li WHERE LocationsT.ID = %li", 
			m_managed.GetCheck(), varCurSelVal.lVal);

		if(m_managed.GetCheck()) {
			//ensure that all administrator users exist in UserLocationT for this location
			_RecordsetPtr rs = CreateRecordset("SELECT PersonID FROM UsersT WHERE Administrator = 1 OR PersonID = %li", BUILT_IN_USER_NEXTECH_TECHSUPPORT_USERID);
			while(!rs->eof) {
				long PersonID = AdoFldLong(rs, "PersonID");
				if(IsRecordsetEmpty("SELECT PersonID FROM UserLocationT WHERE PersonID = %li AND LocationID = %li",PersonID,varCurSelVal.lVal)) {
					ExecuteSql("INSERT INTO UserLocationT (PersonID, LocationID) VALUES (%li, %li)",PersonID,varCurSelVal.lVal);
				}
				rs->MoveNext();
			}
			rs->Close();

			//make sure that ProductLocationInfoT is propagated
			ExecuteSql("INSERT INTO ProductLocationInfoT (ProductID, LocationID, Billable, TrackableStatus) "
				"SELECT ID, %li, "
				"COALESCE((SELECT TOP 1 Billable FROM ProductLocationInfoT WHERE ProductID = ProductT.ID), 0), "
				"COALESCE((SELECT TOP 1 TrackableStatus FROM ProductLocationInfoT WHERE ProductID = ProductT.ID), 0) FROM ProductT",
				varCurSelVal.lVal);

			// (j.gruber 2012-12-04 08:52) - PLID 48566 - and ServiceLocationInfoT
			ExecuteParamSql("INSERT INTO ServiceLocationInfoT (ServiceID, LocationID, ShopFee) "
				"SELECT ID, {INT}, "
				"COALESCE((SELECT TOP 1 ShopFee FROM ServiceLocationInfoT WHERE ServiceLocationInfoT.ServiceID = ServiceT.ID), 0) "
				" FROM ServiceT ",
				varCurSelVal.lVal);
		}
		else {
			
			ExecuteSql("UPDATE LocationsT SET IsDefault = 0 WHERE LocationsT.ID = %li",varCurSelVal.lVal);

			//make sure that ProductLocationInfoT is cleared
			ExecuteSql("DELETE FROM ProductLocationInfoT WHERE LocationID = %li",varCurSelVal.lVal);

			// (j.gruber 2012-12-04 09:56) - PLID 48566 - and servicelocationInfoT
			ExecuteParamSql("DELETE FROM ServiceLocationInfoT WHERE LocationID = {INT}",varCurSelVal.lVal);

			//TES 11/2/2006 - PLID 23314 - Remove any HL7 mappings.
			//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT
			ExecuteSql("DELETE FROM HL7CodeLinkT WHERE Type = %i AND PracticeID = %li", hclrtLocation, varCurSelVal.lVal);
		
		}

		// Network code
		// (a.walling 2007-08-06 12:37) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		CClient::RefreshTable(NetUtils::LocationsT, varCurSelVal.lVal);
		LoadUsers();

		//auditing
		CString strOld, strNew;
		if(IsDlgButtonChecked(IDC_MANAGED)) {
			strNew = "Managed";
			strOld = "Unmanaged";
		}
		else {
			strNew = "Unmanaged";
			strOld = "Managed";
			// (c.haag 2003-07-30 17:44) - Unlink resources linked to this location
			ExecuteSql("DELETE FROM ResourceLocationConnectT WHERE LocationID = %d", varCurSelVal.lVal);
		}
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, strDel, nAuditID, aeiLocationStatus, nDelID, strOld, strNew, aepMedium, aetChanged);

	}
	NxCatchAll("Could not change location setup");

	Load();
}

void CPracticeInfo::OnDblClickCellAllowed(long nRowIndex, short nColIndex) 
{
	if (nRowIndex != -1)
	{	m_disallowed->CurSel = nRowIndex;
		OnRemove();
	}
}

void CPracticeInfo::OnDblClickCellDisallowed(long nRowIndex, short nColIndex) 
{
	if (nRowIndex != -1)
	{	m_allowed->CurSel = nRowIndex;
		OnAdd();
	}
}

void CPracticeInfo::OnAdd() 
{
	IRowSettingsPtr pRow = NULL;

	if (m_list->CurSel == -1 || m_disallowed->CurSel == -1)
		return;
	
	try
	{	ExecuteSql(
			"INSERT INTO UserLocationT (PersonID, LocationID)"
			"SELECT %i, %i", 
			VarLong(m_disallowed->Value[m_disallowed->CurSel][0]), 
			VarLong(m_list->Value[m_list->CurSel][0]));

		pRow = m_disallowed->Row[m_disallowed->CurSel];
		CClient::RefreshTable(NetUtils::Coordinators, VarLong(pRow->GetValue(0)));
		m_allowed->AddRow(pRow);
		m_disallowed->RemoveRow(m_disallowed->CurSel);
		m_disallowed->CurSel = -1;

		return;
	}NxCatchAll("Could not add users to location");

	Load();
}

void CPracticeInfo::OnRemove() 
{
	IRowSettingsPtr pRow = NULL;

	if (m_list->CurSel == -1 || m_allowed->CurSel == -1)
		return;

	try
	{
		//if administrator, do not remove
		if(m_allowed->GetValue(m_allowed->GetCurSel(),2).boolVal) {
			AfxMessageBox("This user is an Administrator, and can log in from all locations.\n"
				"It will not be removed from the list.");
			return;
		}

		//DRT 1/30/03 - Fixed a bug which let you remove all users IF there was a location that was unmanaged and allowed users to login to it
		if(IsRecordsetEmpty("SELECT * FROM UserLocationT INNER JOIN LocationsT ON UserLocationT.LocationID = LocationsT.ID WHERE Managed = 1 AND (PersonID <> %li OR LocationID != %li)",
			VarLong(m_allowed->Value[m_allowed->CurSel][0]), 
			VarLong(m_list->Value[m_list->CurSel][0]))) {

			AfxMessageBox("This operation will disallow any users to log in to Practice.\n"
				"You must have at least one user with the ability to log in.");

			return;
		}

		ExecuteSql("DELETE FROM UserLocationT WHERE PersonID = %li AND LocationID = %li",
			VarLong(m_allowed->Value[m_allowed->CurSel][0]), 
			VarLong(m_list->Value[m_list->CurSel][0]));
		
		pRow = m_allowed->Row[m_allowed->CurSel];
		CClient::RefreshTable(NetUtils::Coordinators, VarLong(pRow->GetValue(0)));
		m_disallowed->AddRow(pRow);
		m_allowed->RemoveRow(m_allowed->CurSel);
		m_allowed->CurSel = -1;
		
		return;
	}NxCatchAll("Could not remove users from location");

	Load();
}

void CPracticeInfo::OnAddAll() 
{
	IRowSettingsPtr pRow = NULL;
	long id;

	if (m_list->CurSel == -1)
		return;

	try
	{	for (int i = 0; i < m_disallowed->GetRowCount(); i++)
		{	pRow = m_disallowed->Row[i];
			CClient::RefreshTable(NetUtils::Coordinators, VarLong(pRow->GetValue(0)));		
			m_allowed->AddRow(pRow);
		}
		m_disallowed->Clear();
		id = VarLong(m_list->Value[m_list->CurSel][0]);

		ExecuteSql(
			"INSERT INTO UserLocationT (PersonID, LocationID)"
			"SELECT PersonID, %i FROM UsersT "
			"WHERE PersonID NOT IN ("
				"SELECT PersonID "
				"FROM UserLocationT "
				"WHERE LocationID = %i "
			")", id, id);

		return;
	}NxCatchAll("Could not add users to location");

	Load();
}

void CPracticeInfo::OnRemoveAll() 
{
	if (m_list->CurSel == -1)
		return;

	try
	{
		//DRT 1/30/03 - Fixed a bug which let you remove all users IF there was a location that was unmanaged and allowed users to login to it
		if(IsRecordsetEmpty("SELECT * FROM UserLocationT  INNER JOIN LocationsT ON UserLocationT.LocationID = LocationsT.ID WHERE Managed = 1 AND LocationID != %li",
			VarLong(m_list->Value[m_list->CurSel][0]))) {

			AfxMessageBox("This operation will disallow any users to log in to Practice.\n"
				"You must have at least one user with the ability to log in.");
			return;
		}

		BOOL bAdministrators = FALSE;

		for (int i = m_allowed->GetRowCount() - 1; i >= 0; i--) {
			if(!m_allowed->GetValue(i,2).boolVal) {
				CClient::RefreshTable(NetUtils::Coordinators, VarLong(m_allowed->GetValue(i,0)));
				m_disallowed->TakeRow(m_allowed->GetRow(i));
			}
			else {
				bAdministrators = TRUE;
			}
		}

		if(bAdministrators)
			AfxMessageBox("Administrator users can log in from any location, and therefore were not removed from the list.");

		ExecuteSql("DELETE FROM UserLocationT WHERE LocationID = %i AND PersonID NOT IN (SELECT PersonID FROM UsersT WHERE Administrator = 1 OR PersonID = -1)", 
			VarLong(m_list->Value[m_list->CurSel][0]));
		return;
	}NxCatchAll("Could not remove users from location");

	Load();
}

void CPracticeInfo::OnEditPlaceOfService() 
{
	try{
		_variant_t var;
		if(m_placeOfService->CurSel!=-1) {
			var = m_placeOfService->GetValue(m_placeOfService->GetCurSel(),0);
		}

		CPlaceCodeDlg dlg(this);
		dlg.DoModal();

		m_placeOfService->Requery();

		if(var.vt!=VT_NULL)
			m_placeOfService->SetSelByColumn(0,var);
	}NxCatchAll("Error in setting selection.");
}

void CPracticeInfo::OnSelChosenLocation(long nRow) 
{
	if(nRow == -1)
		m_list->CurSel = 0;

	m_CurSelect = m_list->GetCurSel();
	if (m_CurSelect != -1)
		Load();
	else
		EnsureButtons();
}

void CPracticeInfo::OnSelChosenProvider(long nRow) 
{
	CString tmpStr;
	if(nRow==-1)
		return;

	if(m_list->GetCurSel() == -1)
		return;

	_variant_t  varCurSelVal;
	//get the id of the current selected location
	varCurSelVal = m_list->GetValue(m_list->GetCurSel(), 0);
	//if the variant is not a long then give a message box because that is the 
	//value we were expecting and that is the value we need it to be
	if (varCurSelVal.vt == VT_I4) {
		_variant_t  varCurSelProvVal;
		//get the id of the default provider of the location
		varCurSelProvVal = m_provider->GetValue(m_provider->GetCurSel(), 0);
		//if the variant is not a long then give a message box because we were exapecting
		//a long and that is what type we need in order to continue
		if (varCurSelProvVal.vt == VT_I4) {
			if(VarLong(varCurSelProvVal) == -1) {
				m_provider->CurSel = -1;
				ExecuteSql("UPDATE LocationsT SET LocationsT.DefaultProviderID = NULL WHERE LocationsT.ID = %li", VarLong(varCurSelVal));
			}
			else {
				//set the default provider to be whatever they selected
				ExecuteSql("Update LocationsT SET LocationsT.DefaultProviderID = %li WHERE LocationsT.ID = %li", varCurSelProvVal.lVal, varCurSelVal.lVal);
			}
		}
		else {//the datalist returned something we can't use
			MsgBox(MB_ICONSTOP|MB_OK, "Error 4423 in PracticeInfo.cpp: OnSelChangedNxdlproviders \n No Valid Selection:Provider DataList");
		}

	}
	else {//the datalist returned something we can't use
		MsgBox(MB_ICONSTOP|MB_OK, "Error 4545 in PracticeInfo.cpp: OnSelChangedNxdlproviders \n No Valid Selection:Location DataList");
	}	
}

void CPracticeInfo::OnSelChosenPlaceOfService(long nRow) 
{
	if(nRow==-1)
		return;

	if(m_list->GetCurSel() == -1)
		return;

	if(m_list->GetValue(m_list->GetCurSel(),0).vt != VT_I4)
		return;
	try
	{
		ExecuteSql("UPDATE LocationsT SET POSID = %li "
			"WHERE ID = %li",
			VarLong(m_placeOfService->Value[nRow][0]),
			VarLong(m_list->Value[m_list->CurSel][0]));
	}
	NxCatchAll("Error in changing Place Of Service");		
}

void CPracticeInfo::OnSelChosenCoordinator(long nRow) 
{
	if(nRow==-1)
		return;

	if(m_list->GetCurSel() == -1)
		return;

	CString tmpStr;
	_variant_t  varCurSelVal;

	try
	{
		long id = VarLong(m_list->Value[m_list->CurSel][0]);
		long coordID = VarLong(m_coordinator->Value[m_coordinator->CurSel][0]);

		if(coordID == -1) {
			m_coordinator->CurSel = -1;
			ExecuteSql("UPDATE LocationsT SET DefaultCoordinatorID = NULL WHERE ID = %li", id);
		}
		else {
			ExecuteSql("UPDATE LocationsT SET DefaultCoordinatorID = %li "
				"WHERE ID = %li", coordID, id);
		}
	}
	NxCatchAll("Could not set default coordinator");
}



bool CPracticeInfo::SaveAreaCode(long nID) {

	//is the member variable empty
	if (m_strAreaCode.IsEmpty() ) {
		//default to returning true becuase just becauase we didn't do anything with the areacode, doesn't mean they didn't change the number
		return true;
	}
	else {
		//check to see if that is the only thing that is in the box
		CString strPhone;
		GetDlgItemText(nID, strPhone);
		if (strPhone == m_strAreaCode) {
			//if they are equal then erase the area code
			SetDlgItemText(nID, "");
			return false;
		}
		else {
			return true;
		}

	}
	//set out member variable to blank
	m_strAreaCode = "";

}

void CPracticeInfo::FillAreaCode(long nID)  {

	
		//first check to see if anything is in this box
		CString strPhone;
		GetDlgItemText(nID, strPhone);
		if (! ContainsDigit(strPhone)) {
			// (j.gruber 2009-10-07 17:23) - PLID 35826 - updated for city lookup
			CString strAreaCode, strZip, strCity;
			GetDlgItemText(IDC_PRACZIP, strZip);
			GetDlgItemText(IDC_PRACCITY, strCity);
			BOOL bResult = FALSE;
			if (!m_bLookupByCity) {
				bResult = GetZipInfo(strZip, NULL, NULL, &strAreaCode);
			}
			else {
				bResult = GetCityInfo(strCity, NULL, NULL, &strAreaCode);
			}
			if (bResult) {
				SetDlgItemText(nID, strAreaCode);
				
				//set the member variable 
				m_strAreaCode.Format("(%s) ###-####", strAreaCode);


				//set the cursor
				::PostMessage(GetDlgItem(nID)->GetSafeHwnd(), EM_SETSEL, 5, 5);
			}
			else {
				//set the member variable to be blank
				m_strAreaCode = "";
			}
			
	  	}
		else {

			//set the member variable to be blank
			m_strAreaCode = "";
		}
}

void CPracticeInfo::OnPracticeLeft() 
{
	//Assess what new location should be displayed	
	try {
		long nCurrent;
		nCurrent = m_list->CurSel;
		if(nCurrent == -1){
			m_list->CurSel = m_list->GetRowCount() - 1;
		}	
		else if(nCurrent > 0){
			m_list->CurSel = nCurrent - 1;
		}
		
		//display the new location
		OnSelChosenLocation(m_list->CurSel);
		EnsureButtons();
				
	}NxCatchAll("Error in CPracticeInfo::OnPracticeRight()");
}

void CPracticeInfo::OnPracticeRight() 
{
	//Assess what new location should be displayed+
	try {
		long nCurrent;
		nCurrent = m_list->CurSel;
		if(nCurrent == -1){
			m_list->CurSel = 0;
		}
		else if(nCurrent < m_list->GetRowCount() - 1){
			m_list->CurSel = nCurrent + 1;
		}
				
		//display the new location
		OnSelChosenLocation(m_list->CurSel);
		EnsureButtons();
	
	}NxCatchAll("Error in CPracticeInfo::OnPracticeRight()");
}


void CPracticeInfo::EnsureButtons()
{
	try {
		//disable the left arrow if the selection is less then 0
		if(m_list->CurSel < 1) {
			m_btnPracticeLeft.EnableWindow(FALSE);
		}
		else{
			m_btnPracticeLeft.EnableWindow(TRUE);
		}
		
		//disable the right arrow if the selection is the last in the list
		if(m_list->CurSel == m_list->GetRowCount()-1) {
			m_btnPracticeRight.EnableWindow(FALSE);
		}
		else {
			m_btnPracticeRight.EnableWindow(TRUE);
		}
			
	}NxCatchAll("Error in CPracticeInfo::EnsureButtons");
}

void CPracticeInfo::OnLocationActive() 
{
	CString tmpStr;
	_variant_t  varCurSelVal;

	if(m_list->GetCurSel() == -1)
		return;
	
	//get the id of the currently selected location
	varCurSelVal = m_list->GetValue(m_list->GetCurSel(), 0);

	//check to see that the value returned from the datalist was a long because if it isn't,
	//then we can't use it so give an error
	if (varCurSelVal.vt != VT_I4) 
		return;

	try
	{
		if(GetCurrentLocation() == VarLong(m_list->GetValue(m_list->CurSel, 0)))
		{
			MsgBox(MB_ICONWARNING|MB_OK,"You cannot change the 'Active' status for a location you are logged into.");
			if (m_buActive.GetCheck())
				m_buActive.SetCheck(FALSE);
			else
				m_buActive.SetCheck(TRUE);
			return;
		}

		//for auditing
		long nDelID = VarLong(m_list->GetValue(m_list->CurSel, 0));
		CString strDel = CString(m_list->GetValue(m_list->CurSel, 1).bstrVal);

		BOOL bIsActiveNow = !m_buActive.GetCheck();
		CString strWarning;
		if(bIsActiveNow) {
			strWarning = "If you make this location Inactive, it will no longer available for logging in, or any other part of the program except the Reports module.  Are you sure you wish to do this?";
		}
		else {
			strWarning = "Are you sure you wish to re-activate this location?";
		}
		if (IDYES != MsgBox(MB_YESNO,
					"%s", strWarning))
		{
			if (m_buActive.GetCheck())
				m_buActive.SetCheck(FALSE);
			else m_buActive.SetCheck(TRUE);

			Load();
			return;		
		}

		long nNewResourceLocation = -1;
		// (c.haag 2003-10-08 10:01) - If the location is linked with any resources, tell the user, and give them the option to back off.
		if (bIsActiveNow)
		{
			CString strWarning = "The following resources are linked with this location:\r\n\r\n";
			BOOL bShowWarning = FALSE;
			_RecordsetPtr prsResources = CreateRecordset("SELECT Item FROM ResourceLocationConnectT LEFT JOIN ResourceT ON ResourceLocationConnectT.ResourceID = ResourceT.ID WHERE LocationID = %d",
				varCurSelVal.lVal);
			while (!prsResources->eof)
			{
				if (!bShowWarning) bShowWarning = TRUE;
				CString str;
				str.Format("%s\r\n", AdoFldString(prsResources, "Item", ""));
				strWarning += str;
				prsResources->MoveNext();
			}
			if (bShowWarning)
			{
				strWarning += "\r\nInactivating this location will unlink it from those resources. Please select a new location for these resources.";
				CSelectLocationDlg dlg(this);
				dlg.m_strCaption = strWarning;
				dlg.m_nCurrentLocationID = VarLong(m_list->Value[m_list->CurSel][0]);
				if (IDOK != dlg.DoModal())
				{
					if (m_buActive.GetCheck())
						m_buActive.SetCheck(FALSE);
					else m_buActive.SetCheck(TRUE);

					Load();
					return;
				}
				else {
					nNewResourceLocation = dlg.m_nNewLocationID;
				}
			}

			// (j.jones 2008-10-08 17:32) - PLID 31596 - disallow changing the type if it is a favorite pharmacy
			if(!IsRecordsetEmpty("SELECT PharmacyID FROM FavoritePharmaciesT WHERE PharmacyID = %li", VarLong(varCurSelVal))) {

				if(IDNO == MessageBox("This location is marked as a favorite pharmacy for patients. "
					"Marking this location inactive will remove this pharmacy from the favorite lists.\n\n"
					"Are you sure you wish to continue?", "Practice", MB_YESNO|MB_ICONEXCLAMATION)) {

					if (m_buActive.GetCheck()) {
						m_buActive.SetCheck(FALSE);
					}
					else {
						m_buActive.SetCheck(TRUE);
					}

					Load();
					return;	
				}
			}
		}

		//JMJ 3/10/03 - by replicating code from OnRemove(), these checks ensure that in no way can you configure the system where nobody can log in

		//first check that there will be at least one managed location
		if(IsRecordsetEmpty("SELECT ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND ID != %li",
			VarLong(m_list->Value[m_list->CurSel][0])) && m_managed.GetCheck() && !m_buActive.GetCheck()) {
			AfxMessageBox("This is the only active, managed location.\n"
				"Disabling this will mean that nobody can log in to Practice from any location.\n"
				"The change will not be made.",MB_ICONEXCLAMATION);

			m_buActive.SetCheck(TRUE);
			return;
		}

		//now check to make sure that there are not going to be any locations that users can't log in to
		//NOTE: technically this query could/should replace the above check, however I (JMJ) feel that the above check
		//is faster and can give a more accurate answer to the user as to what the heck is going on. 
		//Keep in mind that unchecking the last managed location is more common than the following problem:
		if(IsRecordsetEmpty("SELECT PersonID FROM UserLocationT INNER JOIN LocationsT ON UserLocationT.LocationID = LocationsT.ID WHERE Managed = 1 AND Active = 1 AND LocationID != %li",
			VarLong(m_list->Value[m_list->CurSel][0]))) {

			AfxMessageBox("This operation will disallow any users to log in to Practice.\n"
				"You must have at least one user with the ability to log in.\n\n"
				"If you still wish to inactivate this location, select another managed location \n"
				"and allow users to log in to it.");

			m_buActive.SetCheck(TRUE);
			return;
		}

		
		if (bIsActiveNow) {
			if(nNewResourceLocation != -1) {
				ExecuteSql("UPDATE ResourceLocationConnectT SET LocationID = %li WHERE LocationID = %li", nNewResourceLocation, VarLong(varCurSelVal));
			}
			// (c.haag 2003-07-30 17:44) - Unlink resources linked to this location
			ExecuteSql("DELETE FROM ResourceLocationConnectT WHERE LocationID = %d", varCurSelVal.lVal);

			ExecuteSql("UPDATE LocationsT SET IsDefault = 0 WHERE LocationsT.ID = %li",varCurSelVal.lVal);

			// (z.manning 2010-01-11 12:19) - PLID 24044 - Clear out any uses of this as a default lab
			ExecuteParamSql("UPDATE LocationsT SET DefaultLabID = NULL WHERE DefaultLabID = {INT}", varCurSelVal.lVal);
			NXDATALIST2Lib::IRowSettingsPtr pLabRow = m_pdlDefaultLab->FindByColumn(dlccID, varCurSelVal.lVal, NULL, VARIANT_FALSE);
			if(pLabRow != NULL) {
				m_pdlDefaultLab->RemoveRow(pLabRow);
			}

			// (j.jones 2008-10-07 17:16) - PLID 31596 - remove favorite pharmacies
			CString strSql;
			RemoveFavoritePharmacy(strSql, VarLong(varCurSelVal));
			ExecuteSqlBatch(strSql);
		}

		ExecuteSql("Update LocationsT SET LocationsT.Active = %li WHERE LocationsT.ID = %li", 
			m_buActive.GetCheck(), varCurSelVal.lVal);
		// Network code
		// (a.walling 2007-08-06 12:37) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		CClient::RefreshTable(NetUtils::LocationsT, varCurSelVal.lVal);
		LoadUsers();

		//auditing
		CString strOld, strNew;
		if(IsDlgButtonChecked(IDC_LOCATION_ACTIVE)) {
			strNew = "Active";
			strOld = "Inactive";
		}
		else {
			// (b.cardillo 2010-04-06 11:25) - PLID 37097 - Audit the new state as inactive, instead the other way around
			strNew = "Inactive";
			strOld = "Active";
		}
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		if(nAuditID != -1)
			AuditEvent(-1, strDel, nAuditID, aeiLocationStatus, nDelID, strOld, strNew, aepMedium, aetChanged);

		// (v.maida 2016-01-22 10:32) - PLID 68033 - If the location has been inactivated, check if the location is set to a default FFA 
		// location preference and, if so, reset the preference to be the default preference value, since inactive locations are not allowed as defaults.
		if (strNew.CompareNoCase("Inactive") == 0 && UsersHaveLocationAsDefaultFFALocationPref(varCurSelVal.lVal))
		{
			RevertDefaultFFALocationPrefForLocation(varCurSelVal.lVal);
		}

	}
	NxCatchAll("Could not change location setup");

	Load();	
}

void CPracticeInfo::OnCheckDefaultForNewPatients() 
{
	try {

		if(m_list->GetCurSel() == -1)
			return;
		
		//get the id of the currently selected location
		_variant_t varCurSelVal = m_list->GetValue(m_list->GetCurSel(), 0);

		//check to see that the value returned from the datalist was a long because if it isn't,
		//then we can't use it so give an error
		if (varCurSelVal.vt != VT_I4) 
			return;

		long nIsDefault = m_checkDefaultForNewPatients.GetCheck() ? 1 : 0;

		if(nIsDefault) {
			if(IsRecordsetEmpty("SELECT ID FROM LocationsT WHERE Managed = 1 AND Active = 1 AND ID = %li",varCurSelVal.lVal)) {
				AfxMessageBox("The 'default' location must be an active, managed location.");
				m_checkDefaultForNewPatients.SetCheck(FALSE);
				return;
			}

			//remove the default status from any other locations
			ExecuteSql("UPDATE LocationsT SET IsDefault = 0");
		}
		else {
			//TES 5/6/2008 - PLID 29690 - If they're set up to auto-import HL7 patients, they need a default location.  Let
			// them know about that.
			//TES 7/31/2008 - PLID 29690 - This is only a problem if they're set up to auto-import HL7 Patients via TCP/IP,
			// otherwise it's fine because it will just use the currently logged in location.
			//TES 6/22/2011 - PLID 44261 - New method for accessing HL7 Settings.  First, do we have any that batch imports?
			// Don't bother touching HL7 settings, if they're not licensed for HL7
			if (g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent))
			{
				CArray<long, long> arBatchImports;
				GetHL7SettingsGroupsBySetting("BatchImports", FALSE, arBatchImports);
				BOOL bBatchTcpFound = FALSE;
				if (arBatchImports.GetSize()) {
					//TES 6/22/2011 - PLID 44261 - OK, now do any of those have an ImportType of 1?
					CArray<long, long> arTcpImports;
					GetHL7SettingsGroupsBySetting("ImportType", (long)1, arTcpImports);
					for (int i = 0; i < arBatchImports.GetSize() && !bBatchTcpFound; i++) {
						for (int j = 0; j < arTcpImports.GetSize() && !bBatchTcpFound; j++) {
							if (arBatchImports[i] == arTcpImports[j]) bBatchTcpFound = TRUE;
						}
					}
				}
				if (bBatchTcpFound) {
					MsgBox("NOTE: You are configured to automatically import HL7 messages via TCP/IP, but now you do not have a default location "
						"specified for new patients.  Patients cannot be automatically imported without a default location.  "
						"All incoming patients will be batched until a default location is set.");
				}
			}
		}

		ExecuteSql("UPDATE LocationsT SET IsDefault = %li WHERE ID = %li", 
			nIsDefault, varCurSelVal.lVal);
		// Network code

		// (a.walling 2007-08-06 12:37) - PLID 26991 - Send the ID to invalidate rather than the whole table (-1 default)
		CClient::RefreshTable(NetUtils::LocationsT, varCurSelVal.lVal);
	}NxCatchAll("Error setting default location.");
}

LRESULT CPracticeInfo::OnTableChanged(WPARAM wParam, LPARAM lParam) {

	try {
		switch(wParam) {
			case NetUtils::Coordinators:
			case NetUtils::Providers: {
				try {
					UpdateView();
				} NxCatchAll("Error in CPracticeInfo::OnTableChanged:Generic");
				break;
			}
		}

	} NxCatchAll("Error in CPracticeInfo::OnTableChanged");

	return 0;
}

// (b.cardillo 2006-11-28 16:30) - PLID 23682 - I spoke with j.gruber and confirmed that 
// she meant to handle the SelChanged event here, not SelChosen, because she's relying on 
// the OnSelChangingLocationType() as a safeguard around this code.
void CPracticeInfo::OnSelChangedLocationType(LPDISPATCH lpOldSel, LPDISPATCH lpNewSel) 
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpNewSel);
		NXDATALIST2Lib::IRowSettingsPtr pOldRow(lpOldSel);
		long nOldType = -1;
		if(pOldRow != NULL) {
			nOldType = VarLong(pOldRow->GetValue(0));
		}

		if (pRow) {

			long nType = VarLong(pRow->GetValue(0));
			long nLocID = VarLong(m_list->GetValue(m_list->CurSel, 0));

			CString strSql = BeginSqlBatch();
			//get rid a few settings the user has no control over
			AddStatementToSqlBatch(strSql, "DELETE FROM UserLocationT WHERE UserLocationT.LocationID = %li", nLocID);
			AddStatementToSqlBatch(strSql, "DELETE FROM ConfigRT WHERE NAME = 'CurResourceView' AND Number = %li", nLocID);
			AddStatementToSqlBatch(strSql, "DELETE FROM ConfigBillColumnsT WHERE LocationID = %li",nLocID);

			//TES 2/23/2010 - PLID 37501 - Anything that uses HL7LocationLinkT counts on the location being a specific type.  So, if we're
			// changing the type, we can't use this ID any more in any HL7 mapping.
			//TES 5/24/2010 - PLID 38865 - HL7LocationLinkT is now HL7CodeLinkT
			AddStatementToSqlBatch(strSql, "DELETE FROM HL7CodeLinkT WHERE Type = %i AND PracticeID = %li", hclrtLocation, nLocID);

			// (j.jones 2008-10-07 17:16) - PLID 31596 - remove favorite pharmacies
			RemoveFavoritePharmacy(strSql, nLocID);

			//TES 2/1/2010 - PLID 37143 - Clear out any custom Lab Request forms, if this is no longer a lab.
			//TES 7/27/2012 - PLID 51849 - Same for Lab Results forms
			if(nType != 2) {
				AddStatementToSqlBatch(strSql, "DELETE FROM LocationCustomReportsT WHERE ReportID IN (567,658) AND LocationID = %li", nLocID);

				if(nOldType == 2) {
					// (z.manning 2010-02-10 17:27) - PLID 24044 - If it used to be a lab and now isn't
					// then make sure to remove this row from the default lab combo
					NXDATALIST2Lib::IRowSettingsPtr pDefaultLabRow = m_pdlDefaultLab->FindByColumn(dlccID, nLocID, NULL, VARIANT_FALSE);
					if(pDefaultLabRow != NULL) {
						m_pdlDefaultLab->RemoveRow(pDefaultLabRow);
					}
				}
			}
			else {
				// (j.jones 2010-06-25 11:53) - PLID 39185 - link with all labs to be ordered records that aren't already linked
				AddStatementToSqlBatch(strSql, "INSERT INTO LabsToBeOrderedLocationLinkT (LocationID, LabsToBeOrderedID) "
					"SELECT %li, ID FROM LabsToBeOrderedT "
					"WHERE ID NOT IN (SELECT LabsToBeOrderedID FROM LabsToBeOrderedLocationLinkT WHERE LocationID = %li)", nLocID, nLocID);
			}

			if(nType != 1) {
				//TES 7/26/2010 - PLID 39445 - Make sure this location isn't the default for any resource views.
				AddStatementToSqlBatch(strSql, "UPDATE ResourceViewsT SET LocationID = NULL WHERE LocationID = %li", nLocID);
			}

			AddStatementToSqlBatch(strSql, "UPDATE LocationsT SET TypeID = %li WHERE ID = %li", nType, nLocID);

			// (b.cardillo 2006-11-28 17:14) - PLID 23686 - Made it call ExecuteSqlBatch() 
			// here instead of ExecuteSqlStd(), so that the batched statements would be 
			// surrounded by a BEGIN/COMMIT TRAN.  This way, if anything fails, the actual 
			// transaction will be rolled back.
			ExecuteSqlBatch(strSql);

			if(nType == 1 && nOldType != 1) {
				//TES 8/4/2011 - PLID 44862 - If this is now a General-type location, add a user-defined permission object.
				LPCTSTR strDescription = "Controls access to view or create Labs for this location.";
				CString strLocName;
				GetDlgItemText(IDC_NAME, strLocName);
				long nNewSecurityObjectID = AddUserDefinedPermissionToData(nLocID, sptView, strLocName, strDescription, bioLabLocations, sptView);
				AddUserDefinedPermissionToMemory(nNewSecurityObjectID, nLocID, sptView, strLocName, strDescription, bioLabLocations, sptView);
			}
			else if (nOldType == 1 && nType != 1) {
				//TES 8/4/2011 - PLID 44862 - If this used to be a General-type location, remove its user-defined permission object.
				long nSecurityObjectID = DeleteUserDefinedPermissionFromData(bioLabLocations, nLocID);
				DeleteUserDefinedPermissionFromMemory(nSecurityObjectID);
			}

			//DRT 11/19/2008 - PLID 32081 - Update the interface elements based on type
			EnsureTypeSpecificElements(nType);
		}
	}NxCatchAll("Error Setting Type");
	
}

void CPracticeInfo::OnSelChangingLocationType(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel) 
{
	// (z.manning 2010-02-10 17:02) - PLID 24044 - This is to workaround an issue
	// where the selection change event was being fired a 2nd time whenever
	// the selection changed. (Really, since this list is a combo it should be
	// using OnSelChosen instead of OnSelChanged.)
	::SetFocus(NULL);

	//if they are selecting lab then make sure they don't have anything associated with this procedure already
	NXDATALIST2Lib::IRowSettingsPtr pNewSel(*lppNewSel);
	NXDATALIST2Lib::IRowSettingsPtr pOldSel(lpOldSel);

	long nLocationID = VarLong(m_list->GetValue(m_list->CurSel, 0));

	// (b.cardillo 2006-11-28 15:15) - PLID 23265 - Throughout this function I changed every instance 
	// of "*lppNewSel = lpOldSel" to "SafeSetCOMPointer(lppNewSel, lpOldSel)" so that the reference 
	// counting would be done properly.

	if (*lppNewSel == NULL) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}
	//see if they are equal
	else if (VarLong(pNewSel->GetValue(0)) == VarLong(pOldSel->GetValue(0))) {
		SafeSetCOMPointer(lppNewSel, lpOldSel);
	}
	else if (VarLong(pNewSel->GetValue(0)) == 2 || VarLong(pNewSel->GetValue(0)) == 3) {

		//check to see if this location is managed
		_RecordsetPtr rsCheck = CreateRecordset("SELECT Managed FROM LocationsT WHERE ID = %li", VarLong(m_list->GetValue(m_list->CurSel, 0)));
		if (!rsCheck->eof) {
			BOOL bManaged = AdoFldBool(rsCheck, "Managed");
			if (bManaged != 0) {
				// (c.haag 2006-08-14 12:23) - PLID 21964 - We now have a different message for pharmacys
				if (VarLong(pNewSel->GetValue(0)) == 2) {
					MsgBox("This location is managed.  You cannot have a lab location be a managed location; please uncheck the managed checkbox before making this location a lab.");
				} else {
					MsgBox("This location is managed.  You cannot have a pharmacy location be a managed location; please uncheck the managed checkbox before making this location a pharmacy.");
				}
				SafeSetCOMPointer(lppNewSel, lpOldSel);
				return;
			}
		}
		else {
			ASSERT(FALSE);
		}

		//now check to see if anything else is associated with this location - Copied from delete location
		// (e.lally 2005-5-17 16:00) - Do not allow location to be deleted if Persons are
		//		still set to that location.
		_RecordsetPtr prsPersons = CreateRecordset("SELECT top 1 ID "
			"FROM PersonT WHERE location= %li", nLocationID);
		if(!prsPersons->eof)
		{
			MsgBox(MB_ICONWARNING|MB_OK,"This location has patients or contacts associated with it.\n"
					"Please reassign associated patients to a different location.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		//DRT 3/16/2006 - PLID 19749 - You are not allowed to delete a location tied to a prescription.
		if(ReturnsRecords("SELECT TOP 1 PharmacyID FROM PatientMedications WHERE PharmacyID = %li", nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK, "You may not change this location's type because it has prescriptions tied to it. ");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		_RecordsetPtr rs = CreateRecordset("SELECT Count(ID) "
			"FROM BillsT "
			"WHERE Location = %li", 
			nLocationID);
		if (!(rs->eof && rs->bof || rs->Fields->Item[0L]->Value.lVal == 0))
		{	MsgBox(MB_ICONWARNING|MB_OK, 
				"You may not change the type of this location because it has a corresponding bill.\n"
				"Please delete the bill before attempting to change this location's type.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
		rs->Close();

		if (ReturnsRecords("SELECT ID FROM ProductLocationTransfersT WHERE SourceLocationID = %li OR DestLocationID = %li",nLocationID, nLocationID))
		{	MsgBox(MB_ICONWARNING|MB_OK, 
				"You may not change this location's type because it has been involved in an inventory transfer.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		//if there are no orders or product adjustments then delete from product location infoT

		if (ReturnsRecords("SELECT ID FROM ProductAdjustmentsT WHERE LocationID = %li", nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK, 
				"You may not change this location's type because it has a product adjustment associated with it in inventory");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		// (j.gruber 2008-03-03 16:57) - PLID 28955 - include the sold to location as well
		if (ReturnsRecords("SELECT ID FROM OrderT WHERE LocationID = %li OR LocationSoldTo = %li", nLocationID, nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it has an order associated with it in inventory");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		// (j.gruber 2007-02-26 09:19) - 24916 - ensure you cannot change a location used on batch payments
		if(ReturnsRecords("SELECT TOP 1 ID FROM BatchPaymentsT WHERE Location = %li", nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK, "You may not change this location's type because it has batch payments associated with it.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		// (j.gruber 2007-02-26 09:55) - 24931 - ensure you cannot change a location used on lineitems
		if(ReturnsRecords("SELECT TOP 1 ID FROM LineItemT WHERE LocationID = %li", nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK, "You may not change this location's type because it has charges or payments associated with it.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}



		if(!IsRecordsetEmpty("SELECT ID FROM AppointmentsT WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK, 
				"You may not change this location's type because it has a corresponding appointment in the scheduler.\n"
				"Please delete the appointment before attempting change this location's type.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		if(!IsRecordsetEmpty("SELECT ID FROM EMRMasterT WHERE LocationID = %li", nLocationID)){
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it has at least one corresponding EMR.\n"
				"Please change the location of the EMR before attempting to change this location's type.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		//DRT 6/30/2006 - PLID 21291 - Do not allow deleting locations tied to labs (as either the lab or the practice location)
		if(ReturnsRecords("SELECT ID FROM LabsT WHERE LocationID = %li ", nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change a location's type that is tied to a lab entry.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		// (z.manning, 06/05/2007) - PLID 14238 - Don't let them chage a location type if it's tied to a NexPDA profile.
		// (z.manning 2009-11-12 15:35) - PLID 31879 - May also be NexSync
		if(ReturnsRecords("SELECT TOP 1 LocationID FROM OutlookCalendarT WHERE LocationID = %li", nLocationID)) {
			MessageBox("This location is tied to NexPDA and/or NexSync profile(s) and may not be deleted. Please mark the location inactive instead.");
			// (z.manning 2009-11-12 16:03) - PLID 36284 - Forgot the call to SafeSetCOMPointer here
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		//TES 6/21/2010 - PLID 5888 - Don't delete locations that are used in Resource Availability templates
		if (ReturnsRecords("SELECT LocationID FROM ResourceAvailTemplateT WHERE LocationID = %d", nLocationID))
		{
			//(e.lally 2010-07-15) PLID 39626 - Renamed to Location Templates
			MsgBox("You may not change this location's type, because it is used on one or more Location Templates.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		CString strWarning = "The following resources are linked with this location:\r\n\r\n";
		BOOL bShowWarning = FALSE;
		_RecordsetPtr prsResources = CreateRecordset("SELECT Item FROM ResourceLocationConnectT LEFT JOIN ResourceT ON ResourceLocationConnectT.ResourceID = ResourceT.ID WHERE LocationID = %d",
			nLocationID);
		while (!prsResources->eof)
		{
			if (!bShowWarning)
				bShowWarning = TRUE;
			CString str;
			str.Format("%s\r\n", AdoFldString(prsResources, "Item", ""));
			strWarning += str;
			prsResources->MoveNext();
		}
		if (bShowWarning)
		{
			strWarning += "\r\nChanging this location's type will result in no location being linked with these resources.  You may not change this location's type.";
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
			
		}
		prsResources->Close();
		prsResources.Release();

		// (c.haag 2004-12-28 13:08) - Ask about multi-fee information
		// (d.lange 2015-10-02 10:19) - PLID 67117 - Rename from fee groups to fee schedules
		strWarning = "The following fee schedules are linked with this location:\r\n\r\n";
		prsResources = CreateRecordset("SELECT Name FROM MultiFeeLocationsT LEFT JOIN MultiFeeGroupsT ON MultiFeeLocationsT.FeeGroupID = MultiFeeGroupsT.ID WHERE LocationID = %d",
			nLocationID);
		bShowWarning = FALSE;
		while (!prsResources->eof)
		{
			if (!bShowWarning)
				bShowWarning = TRUE;
			CString str;
			str.Format("%s\r\n", AdoFldString(prsResources, "Name", ""));
			strWarning += str;
			prsResources->MoveNext();
		}
		if (bShowWarning)
		{
			strWarning += "\r\nDo you still wish to change this location's type?";
			if (IDYES != MsgBox(MB_YESNO, strWarning))
			{
				SafeSetCOMPointer(lppNewSel, lpOldSel);
				return;
			}
		}

		//TES 9/8/2008 - PLID 27727 - Dropped DefLocationID
		/*if(!IsRecordsetEmpty("SELECT PersonID FROM ProvidersT WHERE DefLocationID = %li",nLocationID)){
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is the default location for at least one Provider.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}*/

		if(!IsRecordsetEmpty("SELECT LocationID FROM ProductLocationInfoT WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is connected to products.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		// (j.gruber 2012-12-04 09:57) - PLID 48566 - and serviceinfolocationT
		if(!IsRecordsetEmpty("SELECT LocationID FROM ServiceLocationInfoT WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is connected to services.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		if(!IsRecordsetEmpty("SELECT LocationID FROM ProductResponsibilityT WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because there is a default ordering user assigned to it.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
		
		if(!IsRecordsetEmpty("SELECT LocationID FROM PatientMedications WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is attached to at least one prescription");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		// (j.jones 2008-10-08 17:32) - PLID 31596 - disallow changing the type if it is a favorite pharmacy
		if(!IsRecordsetEmpty("SELECT PharmacyID FROM FavoritePharmaciesT WHERE PharmacyID = %li", nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is marked as a favorite pharmacy for patients.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		if(!IsRecordsetEmpty("SELECT LocationID FROM InsuranceReferralsT WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is associated with at least one insurance referral.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		if(!IsRecordsetEmpty("SELECT LocationID FROM MarketingCostsT WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is associated with at least one marketing cost.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		if(!IsRecordsetEmpty("SELECT LocationID FROM InsuranceFacilityID WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is associated with at least one insurance facility.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}		
		
		if(!IsRecordsetEmpty("SELECT LocationID FROM AdvHCFAPinT WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is associated with at least one HCFA group setup.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}		
		
		if(!IsRecordsetEmpty("SELECT LocationID FROM POSLocationLinkT WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is associated with at least one place of service.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}		

		if(!IsRecordsetEmpty("SELECT LocationID FROM ResourceLocationConnectT WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is associated with at least one resource.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}		
		
		if(!IsRecordsetEmpty("SELECT LocationID FROM MultiFeeLocationsT WHERE LocationID = %li",nLocationID)) {
			// (d.lange 2015-11-02 16:16) - PLID 67117 - Renamed to fee schedule
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is associated with at least one fee schedule.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		// (j.jones 2007-10-15 14:50) - PLID 27757 - handle the new structure
		if(!IsRecordsetEmpty("SELECT LocationID FROM FacilityFeeSetupT WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is associated with at least one facilty fee.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		if(!IsRecordsetEmpty("SELECT LocationID FROM AnesthesiaSetupT WHERE LocationID = %li",nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change this location's type because it is associated with at least one anesthesia fee.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
			
	}
	else if (VarLong(pNewSel->GetValue(0)) == 1) {

		//they are switching to a regular location, not much to check here
		if(ReturnsRecords("SELECT ID FROM LabsT WHERE LabLocationID = %li ", nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change a location's type that is tied to a lab entry.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

		//they are switching to a regular location, not much to check here
		if(ReturnsRecords("SELECT PharmacyID FROM PatientMedications WHERE PharmacyID = %li ", nLocationID)) {
			MsgBox(MB_ICONWARNING|MB_OK,
				"You may not change a location's type that is tied to a prescription.");
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}

	}

	// (z.manning 2010-02-10 17:19) - PLID 24044 - Old type is lab
	if(VarLong(pOldSel->GetValue(0)) == 2)
	{
		// (z.manning 2010-02-10 17:07) - PLID 24044 - Don't allow changing the type if this is used as a
		// default lab.
		if(ReturnsRecords("SELECT TOP 1 ID FROM LocationsT WHERE DefaultLabID = %li", nLocationID)) {
			MessageBox("This location is used as a default lab so you may not change its type.", NULL, MB_ICONWARNING|MB_OK);
			SafeSetCOMPointer(lppNewSel, lpOldSel);
			return;
		}
	}

}

void CPracticeInfo::OnButtonLocationBiography() 
{
	// (d.moore 2007-07-16 16:21) - PLID 14799 - Added functionality for adding/editing 
	//  biography text associated with a location.
	
	try {
		if (m_list->CurSel == -1) {
			return;
		}
		long nLocationID = VarLong(m_list->GetValue(m_list->CurSel, 0), -1);
		if (nLocationID <= 0) {
			return;
		}
		
		CLocationBiographyDlg dlg(nLocationID, this);
		dlg.DoModal();

	} NxCatchAll("Error In: CPracticeInfo::OnButtonLocationBiography");
}

void CPracticeInfo::OnButtonLocationLogo() 
{
	// (d.moore 2007-07-16 16:21) - PLID 14799 - Added the ability to select a logo for a location.
	
	try {
		if (m_list->CurSel == -1) {
			return;
		}
		long nLocationID = VarLong(m_list->GetValue(m_list->CurSel, 0), -1);
		if (nLocationID <= 0) {
			return;
		}

		CLocationSelectImageDlg dlg(nLocationID, eitLogo, this);
		dlg.DoModal();

	} NxCatchAll("Error In: CPracticeInfo::OnButtonLocationLogo");
}

void CPracticeInfo::OnButtonLocationImage() 
{
	// (d.moore 2007-07-16 16:22) - PLID 14799 - Add the ability to select an image for a location.
	//  This is in addition to the logo for the location. They may want to show an image of the
	//  office building or of the staff.
	
	try {
		if (m_list->CurSel == -1) {
			return;
		}
		long nLocationID = VarLong(m_list->GetValue(m_list->CurSel, 0), -1);
		if (nLocationID <= 0) {
			return;
		}

		CLocationSelectImageDlg dlg(nLocationID, eitGeneralImage, this);
		dlg.DoModal();

	} NxCatchAll("Error In: CPracticeInfo::OnButtonLocationImage");
}

// (j.jones 2008-10-08 17:26) - PLID 31596 - removes favorite pharmacies, and puts the results in a batch statement
void CPracticeInfo::RemoveFavoritePharmacy(CString &strSqlBatch, long nPharmacyID)
{
	try {

		//we need to update order indices when we remove pharmacies
		_RecordsetPtr rsPharmacies = CreateParamRecordset("SELECT PatientID, OrderIndex FROM FavoritePharmaciesT WHERE PharmacyID = {INT}", nPharmacyID);
		while(!rsPharmacies->eof) {
			long nPatientID = AdoFldLong(rsPharmacies, "PatientID");
			long nOrderIndex = AdoFldLong(rsPharmacies, "OrderIndex");
			AddStatementToSqlBatch(strSqlBatch, "UPDATE FavoritePharmaciesT SET OrderIndex = OrderIndex - 1 "
				"WHERE PatientID = %li AND PharmacyID <> %li AND OrderIndex > %li", nPatientID, nPharmacyID, nOrderIndex);
			rsPharmacies->MoveNext();
		}
		rsPharmacies->Close();
		AddStatementToSqlBatch(strSqlBatch, "DELETE FROM FavoritePharmaciesT WHERE PharmacyID = %li", nPharmacyID);

	}NxCatchAll("Error in CPracticeInfo::RemoveFavoritePharmacy");
}

//DRT 11/19/2008 - PLID 32081 - Handle Pharmacy IDs
void CPracticeInfo::OnBnClickedEditPharmacyId()
{
	try {
		//Ensure there's a row selected
		long nRow = m_list->GetCurSel();
		if(nRow == -1) {
			return;
		}

		IRowSettingsPtr pRow = m_list->GetRow(nRow);

		//Open the pharmacy ID dialog with the given ID and Name of the current pharmacy
		CPharmacyIDDlg dlg(this);
		dlg.m_nPharmacyID = VarLong(pRow->GetValue(0));
		dlg.m_strPharmacyName = VarString(pRow->GetValue(1));
		dlg.DoModal();

	} NxCatchAll("Error in OnBnClickedEditPharmacyId");
}

//DRT 11/19/2008 - PLID 32081 - Handle Pharmacy Staff
void CPracticeInfo::OnBnClickedEditPharmacyStaff()
{
	try {
		//Ensure there's a row selected
		long nRow = m_list->GetCurSel();
		if(nRow == -1) {
			return;
		}

		IRowSettingsPtr pRow = m_list->GetRow(nRow);

		//Open the pharmacy staff dialog with the given ID and name of the current pharmacy
		CEditPharmacyStaffDlg dlg(this);
		dlg.m_nLocationID = VarLong(pRow->GetValue(0));
		dlg.m_strPharmacyName = VarString(pRow->GetValue(1));
		dlg.DoModal();

	} NxCatchAll("Error in OnBnClickedEditPharmacyStaff");
}

void CPracticeInfo::EnsureTypeSpecificElements(long nCurrentTypeID)
{
	// (a.walling 2009-03-30 10:45) - PLID 33732 - Do not allow managed labs or pharmacies
	// but if they are already set to managed somehow, we'll allow them to uncheck it.
	if(nCurrentTypeID != 1) {
		// disable these checkboxes (if they are not already checked somehow)
		m_managed.EnableWindow(FALSE);		
		m_checkDefaultForNewPatients.EnableWindow(FALSE);
		// (z.manning 2010-01-11 11:47) - PLID 24044 - Hide the default lab field on general locations
		GetDlgItem(IDC_DEFAULT_LAB)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_DEFAULT_LAB_LABEL)->ShowWindow(SW_HIDE);

		// (b.spivey, March 28, 2012) - PLID 47521 - Doesn't apply to anything other than general... yet. 
		//	 If we expand this functionality, we can just remove the below code so it shows on all location types.
		m_nxeditAbbreviation.ShowWindow(SW_HIDE); 
		m_nxstaticAbbreviationLabel.ShowWindow(SW_HIDE); 

	} else {
		m_managed.EnableWindow(TRUE);
		m_checkDefaultForNewPatients.EnableWindow(TRUE);
		// (z.manning 2010-01-11 11:47) - PLID 24044 - Show the default lab field on general locations if
		// they have a labs license
		if(g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) {
			GetDlgItem(IDC_DEFAULT_LAB)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_DEFAULT_LAB_LABEL)->ShowWindow(SW_SHOW);
		}
		else {
			GetDlgItem(IDC_DEFAULT_LAB)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_DEFAULT_LAB_LABEL)->ShowWindow(SW_HIDE);
		}

		// (b.spivey, March 28, 2012) - PLID 47521 - Doesn't apply to anything other than general... yet. 
		//	 If we expand this functionality, we can just remove the below code so it shows on all location types.
		m_nxeditAbbreviation.ShowWindow(SW_SHOW); 
		m_nxstaticAbbreviationLabel.ShowWindow(SW_SHOW); 
	}

	if(nCurrentTypeID == 3) {
		//Pharmacy
		GetDlgItem(IDC_EDIT_PHARMACY_ID)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_EDIT_PHARMACY_STAFF)->ShowWindow(SW_SHOW);
		// (a.walling 2009-03-30 09:43) - PLID 33729 - Link with directory checkbox
		if (g_pLicense->CheckForLicense(CLicense::lcePrescribe, CLicense::cflrSilent)) {
			GetDlgItem(IDC_LINK_WITH_DIRECTORY)->ShowWindow(SW_SHOW);
		} else {
			GetDlgItem(IDC_LINK_WITH_DIRECTORY)->ShowWindow(SW_HIDE);
		}
	}
	else {
		//Not a pharmacy, hide these
		GetDlgItem(IDC_EDIT_PHARMACY_ID)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_PHARMACY_STAFF)->ShowWindow(SW_HIDE);
		// (a.walling 2009-03-30 09:43) - PLID 33729 - Link with directory checkbox
		GetDlgItem(IDC_LINK_WITH_DIRECTORY)->ShowWindow(SW_HIDE);
	}
}

void CPracticeInfo::OnBnClickedLinkWithDirectory()
{
	try {		
		_variant_t varCurSelVal;
		//get the id of the current selection out of the datalist
		varCurSelVal = m_list->GetValue(m_list->GetCurSel(), 0);
		//check that we have a long because if we don't then we can't use it,
		//so we need to give an error
		if (varCurSelVal.vt == VT_I4) {
			int nState = m_checkLinkWithDirectory.GetCheck();
			CString strValue;

			switch (nState) {
				case BST_UNCHECKED:
					strValue = "0";
					break;
				case BST_CHECKED:
					strValue = "1";
					break;
					/*
				case BST_INDETERMINATE:
					strValue = "NULL";
					break;
					*/
				default:
					ASSERT(FALSE);
					return;
			};

			ExecuteSql("UPDATE LocationsT SET LinkToDirectory = %s WHERE ID = %li", strValue, VarLong(varCurSelVal));
		}

	} NxCatchAll(__FUNCTION__);
}

// (z.manning 2010-01-11 11:38) - PLID 24044
void CPracticeInfo::OnSelChosenDefaultLab(LPDISPATCH lpRow)
{
	try
	{
		_variant_t varDefaultLabID;
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if(pRow != NULL) {
			varDefaultLabID = pRow->GetValue(dlccID);
		}
		else {
			varDefaultLabID = g_cvarNull;
			m_pdlDefaultLab->SetSelByColumn(dlccID, g_cvarNull);
		}

		long nID = VarLong(m_list->GetValue(m_list->GetCurSel(), 0));
		ExecuteParamSql(
			"UPDATE LocationsT SET DefaultLabID = {VT_I4} WHERE ID = {INT}"
			, varDefaultLabID, nID);

	}NxCatchAll(__FUNCTION__);
}


// (s.tullis 2016-03-07 11:56) - PLID 68444 - Save our new selection
void CPracticeInfo::SelChosenLocationDefaultclaimform(LPDISPATCH lpRow)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow) {
			long nClaimID = VarLong(pRow->GetValue(cflID));
			long nID = VarLong(m_list->GetValue(m_list->GetCurSel(), 0));
			ExecuteParamSql(
				"UPDATE LocationsT SET DefaultClaimForm = {INT} WHERE ID = {INT} "
				, nClaimID, nID);
		}
	}NxCatchAll(__FUNCTION__)
}
// (s.tullis 2016-03-07 11:56) - PLID 68444 - Populate the Claim Form list with selections from the bill
void CPracticeInfo::InitClaimFormList()
{
	try {
		if(m_pdlDefaultClaimForm->GetRowCount() > 0)
		{
			return;
		}

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pdlDefaultClaimForm->GetNewRow();
		pRow->PutValue(cflID, (long)1);
		pRow->PutValue(cflName, "HCFA");
		m_pdlDefaultClaimForm->AddRowSorted(pRow, NULL);

		pRow = m_pdlDefaultClaimForm->GetNewRow();
		pRow->PutValue(cflID, (long)2);
		pRow->PutValue(cflName, "UB");
		m_pdlDefaultClaimForm->AddRowSorted(pRow, NULL);

		pRow = m_pdlDefaultClaimForm->GetNewRow();
		pRow->PutValue(cflID, (long)3);
		pRow->PutValue(cflName, "ADA");
		m_pdlDefaultClaimForm->AddRowSorted(pRow, NULL);

		pRow = m_pdlDefaultClaimForm->GetNewRow();
		pRow->PutValue(cflID, (long)4);
		pRow->PutValue(cflName, "IDPA");
		m_pdlDefaultClaimForm->AddRowSorted(pRow, NULL);

		pRow = m_pdlDefaultClaimForm->GetNewRow();
		pRow->PutValue(cflID, (long)5);
		pRow->PutValue(cflName, "NYWC");
		m_pdlDefaultClaimForm->AddRowSorted(pRow, NULL);

		pRow = m_pdlDefaultClaimForm->GetNewRow();
		pRow->PutValue(cflID, (long)6);
		pRow->PutValue(cflName, "MICR");
		m_pdlDefaultClaimForm->AddRowSorted(pRow, NULL);

		pRow = m_pdlDefaultClaimForm->GetNewRow();
		pRow->PutValue(cflID, (long)7);
		pRow->PutValue(cflName, "NY Medicaid");
		m_pdlDefaultClaimForm->AddRowSorted(pRow, NULL);
	}NxCatchAll(__FUNCTION__)
}

// (s.tullis 2016-03-07 11:56) - PLID 68444 -Only Show these controls for managed locations
void CPracticeInfo::ShowHideDefaultClaimFormControls(bool bShow /*= true*/)
{
	try {
		GetDlgItem(IDC_LOCATION_DEFAULTCLAIMFORM)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
		GetDlgItem(IDC__LOCATION_CLAIM_LABEL)->ShowWindow(bShow ? SW_SHOW : SW_HIDE);
	}NxCatchAll(__FUNCTION__)
}