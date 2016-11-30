// NewContact.cpp : implementation file
#include "stdafx.h"
#include "ContactsRc.h"
#include "PracProps.h"
#include "NewContact.h"
#include "UserPropsDlg.h"
#include "GlobalDataUtils.h"
#include "client.h"
#include "duplicatecontact.h"
#include "AuditTrail.h"
#include "CopyPermissionsDlg.h"
#include "DateTimeUtils.h"
#include "PPCLink.h"
#include "AttendanceUtils.h"
#include "ConfigurePermissionGroupsDlg.h"
// (v.maida 2014-12-23 12:19) - PLID 64472 - Added ability to automatically export new referring physicians to HL7.
#include "HL7Utils.h"
#include "GlobalUtils.h"
#include "HL7Client_Practice.h"
// (r.farnworth 2015-12-30 15:31) - PLID 67719 - Need to call API for updating user properties
#include "NxAPI.h"


using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37024 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



/////////////////////////////////////////////////////////////////////////////
// CNewContact dialog


CNewContact::CNewContact(CWnd* pParent)
	: CNxDialog(CNewContact::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewContact)
	//}}AFX_DATA_INIT
	m_bSaveEditEnable = TRUE;
}

void CNewContact::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewContact)
	DDX_Control(pDX, IDC_MALE, m_male);
	DDX_Control(pDX, IDC_FEMALE, m_female);
	DDX_Control(pDX, IDC_MAIN_BTN, m_btnMain);
	DDX_Control(pDX, IDC_REFPHYS_BTN, m_btnRef);
	DDX_Control(pDX, IDC_SUPPLIER_BTN, m_btnSup);
	DDX_Control(pDX, IDC_OTHER_BTN, m_btnOther);
	DDX_Control(pDX, IDC_EMPLOYEE_BTN, m_btnEmp);
	DDX_Control(pDX, IDC_SAVE_AND_EDIT, m_btnSaveEdit);
	DDX_Control(pDX, IDC_SAVE_AND_RESUME, m_btnSaveResume);
	DDX_Control(pDX, IDC_CANCEL, m_btnCancel);
	DDX_Control(pDX, IDC_EMPLOYER_BOX, m_nxeditEmployerBox);
	DDX_Control(pDX, IDC_FIRST_NAME_BOX, m_nxeditFirstNameBox);
	DDX_Control(pDX, IDC_MIDDLE_NAME_BOX, m_nxeditMiddleNameBox);
	DDX_Control(pDX, IDC_LAST_NAME_BOX, m_nxeditLastNameBox);
	DDX_Control(pDX, IDC_ADDRESS1_BOX, m_nxeditAddress1Box);
	DDX_Control(pDX, IDC_ADDRESS2_BOX, m_nxeditAddress2Box);
	DDX_Control(pDX, IDC_ZIP_BOX, m_nxeditZipBox);
	DDX_Control(pDX, IDC_CITY_BOX, m_nxeditCityBox);
	DDX_Control(pDX, IDC_STATE_BOX, m_nxeditStateBox);
	DDX_Control(pDX, IDC_HOME_PHONE_BOX, m_nxeditHomePhoneBox);
	DDX_Control(pDX, IDC_WORK_PHONE_BOX, m_nxeditWorkPhoneBox);
	DDX_Control(pDX, IDC_EXT_PHONE_BOX, m_nxeditExtPhoneBox);
	DDX_Control(pDX, IDC_NOTES, m_nxeditNotes);
	DDX_Control(pDX, IDC_ID_LABEL12, m_nxstaticIdLabel12);
	DDX_Control(pDX, IDC_NPI_BOX, m_nxeditNpi);
	DDX_Control(pDX, IDC_NPI_LABEL, m_nxstaticNpiLabel);
	//(c.copits 2011-09-15) PLID 35359 - Add Fax and Email to new Ref. Physician contact screen.
	DDX_Control(pDX, IDC_FAX_BOX, m_nxeditFaxBox);
	DDX_Control(pDX, IDC_EMAIL_BOX, m_nxeditEmailBox);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CNewContact, CNxDialog)
	//{{AFX_MSG_MAP(CNewContact)
	ON_EN_KILLFOCUS(IDC_ZIP_BOX, OnKillfocusZipBox)
	ON_BN_CLICKED(IDC_SAVE_AND_EDIT, OnSaveAndEdit)
	ON_BN_CLICKED(IDC_SAVE_AND_RESUME, OnSaveAndResume)
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_MAIN_BTN, &CNewContact::OnBnClickedMainBtn)
	ON_BN_CLICKED(IDC_EMPLOYEE_BTN, &CNewContact::OnBnClickedEmployeeBtn)
	ON_BN_CLICKED(IDC_REFPHYS_BTN, &CNewContact::OnBnClickedRefphysBtn)
	ON_BN_CLICKED(IDC_SUPPLIER_BTN, &CNewContact::OnBnClickedSupplierBtn)
	ON_BN_CLICKED(IDC_OTHER_BTN, &CNewContact::OnBnClickedOtherBtn)
	ON_EN_KILLFOCUS(IDC_CITY_BOX, OnKillfocusCityBox)
	//(c.copits 2011-09-22) PLID 45626 - Validate Email Addresses
	ON_EN_KILLFOCUS(IDC_EMAIL_BOX, OnKillfocusEmailBox)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewContact message handlers

BEGIN_EVENTSINK_MAP(CNewContact, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CNewContact)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CNewContact::OnKillfocusZipBox() 
{	
	// (j.gruber 2009-10-07 17:07) - PLID 35826 - updated for city lookup
	if (!m_bLookupByCity) {
		CString city, 
				state,
				tempCity,
				tempState,
				value;
		GetDlgItemText(IDC_ZIP_BOX, value);
		GetDlgItemText(IDC_CITY_BOX, tempCity);
		GetDlgItemText(IDC_STATE_BOX, tempState);
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
			if(city== "" && state== ""){					
				CString str;
				str = value.Left(5);// Get the 5 digit zip code
				GetZipInfo(str, &city, &state);
				// (b.savon 2014-04-03 13:02) - PLID 61644 - If you enter a 9
				//digit zipcode in the locations tab of Administrator, it looks
				//up the city and state based off the 5 digit code, and then 
				//changes the zip code to 5 digits. It should not change the zip code.
			}
			if(tempCity == "") 
				SetDlgItemText(IDC_CITY_BOX, city);
			if(tempState == "")
				SetDlgItemText(IDC_STATE_BOX, state);
		}
	}
}

void CNewContact::OnKillfocusCityBox() 
{	
	try {
		// (j.gruber 2009-10-07 17:07) - PLID 35826 - updated for city lookup
		if (m_bLookupByCity) {
			CString zip, 
					state,
					tempZip,
					tempState,
					value;
			GetDlgItemText(IDC_CITY_BOX, value);
			GetDlgItemText(IDC_ZIP_BOX, tempZip);
			GetDlgItemText(IDC_STATE_BOX, tempState);
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
				if(tempZip == "") 
					SetDlgItemText(IDC_ZIP_BOX, zip);
				if(tempState == "")
					SetDlgItemText(IDC_STATE_BOX, state);
			}
		}
	}NxCatchAll(__FUNCTION__);
}

void PrepareString (CString &str)
{
	str.Replace ("#", " ");
	str.TrimLeft();
	str.TrimRight();
	if (str == "")
		str = "NULL";
	else str = "\"" + str + "\"";
}

bool CNewContact::Save() 
{
	//(c.copits 2011-09-15) PLID 35359 - Add Fax and Email to new Ref. Physician contact screen.
	CString		first, middle, last, add1, add2, city, state, zip, home, work, ext,
				company, memo, sql, fax, email;
	long nGender;

	GetDlgItemText(IDC_LAST_NAME_BOX,	last);
	GetDlgItemText(IDC_EMPLOYER_BOX,	company);
	if (_Q(last) == "" && _Q(company) == "")
	{	MsgBox("Contacts require a name or company");
		return false;
	}
	GetDlgItemText(IDC_FIRST_NAME_BOX,	first);
	GetDlgItemText(IDC_MIDDLE_NAME_BOX, middle);

	//DRT 7/14/03 - We are no longer going to bother checking duplicates if they are just entering a company name
	//		and the first/last/middle are blank, it's just too confusing for the user because they see a long list of
	//		people who are not duplicates.
	if(!(!company.IsEmpty() && last.IsEmpty() && first.IsEmpty() && middle.IsEmpty())) {
	
		CDuplicateContact		dlg;
		int choice;
		if (dlg.FindDuplicates (_Q(first), _Q(last), _Q(middle))) {	
			choice = dlg.DoModal(); // Returns abort, retry or ignore ("cancel", "change name", or "add" respectively)
			if (choice == IDRETRY) {
				return false;
			} else if (choice == IDABORT) {	
				EndDialog(0);
				return false;
			}
		}
	}
	else {
		try {
			//we really do need to make sure they aren't entering duplicate names, so do a quick lookup and prompt them
			_RecordsetPtr prs = CreateRecordset("SELECT Company FROM PersonT WHERE Company = '%s'", _Q(company));

			if(!prs->eof) {
				if(MsgBox(MB_YESNO, "There are %li existing companies with this name.  Are you sure you want to save it?", prs->GetRecordCount()) == IDNO) {
					return false;
				}
			}
			prs->Close();
		} NxCatchAll("Error searching for duplicate companies.");
	}

	// (z.manning 2008-12-05 09:13) - PLID 28277 - Added NPI field
	CString strNpi;
	m_nxeditNpi.GetWindowText(strNpi);
	// (z.manning 2008-12-05 09:21) - PLID 28277 - Also check the NPI
	if(m_btnMain.GetCheck() == BST_CHECKED || m_btnRef.GetCheck() == BST_CHECKED) {
		if(!CheckNPI(strNpi, this, TRUE)) {
			return false;
		}
	}

	
	GetDlgItemText(IDC_ADDRESS1_BOX,	add1);
	GetDlgItemText(IDC_ADDRESS2_BOX,	add2);
	GetDlgItemText(IDC_CITY_BOX,		city);
	GetDlgItemText(IDC_STATE_BOX,		state);
	GetDlgItemText(IDC_ZIP_BOX,			zip);
	GetDlgItemText(IDC_HOME_PHONE_BOX,	home);
	GetDlgItemText(IDC_WORK_PHONE_BOX,	work);
	GetDlgItemText(IDC_EXT_PHONE_BOX,	ext);
	GetDlgItemText(IDC_NOTES,	memo);
	//(c.copits 2011-09-15) PLID 35359 - Add Fax and Email to new Ref. Physician contact screen.
	GetDlgItemText(IDC_FAX_BOX,			fax);
	GetDlgItemText(IDC_EMAIL_BOX,		email);

	if (m_male.GetCheck())
		nGender = 1;
	else if (m_female.GetCheck())
		nGender = 2;
	else nGender = 0;

	EnsureRemoteData();
	//(e.lally) - initializing the newID *just in case*
	long nNewID =-1;
	long AuditItem;

	CString strSaveString = BeginSqlBatch();

	// (b.savon 2012-02-21 15:17) - PLID 48274 - Respect password rules
	//defined outside the if statements because it's referenced in multiple places
	CUserPropsDlg dlg(this);
	dlg.SetNewUser(TRUE);

	try	{

		//TES 9/8/2008 - PLID 27727 - We now just use the currently logged-in location rather than going through all this 
		// rigmarole.  They can change it now, if they want.
		/*long nLocID;
		//check to see how many managed locations there are
		_RecordsetPtr rsLoc;
		// (a.walling 2007-10-11 13:05) - PLID 27726 - Order by ID in case we need to fall back to the first managed (and active) location
		rsLoc = CreateRecordset("SELECT ID From LocationsT WHERE Managed = 1 AND Active = 1 ORDER BY ID ASC");
		long nNumCount = rsLoc->RecordCount;
		if (nNumCount == 1) {
			//get the ID of what we want the location of the contact to be
			nLocID = AdoFldLong(rsLoc, "ID");
		}
		else {
			long nFirstLocID = -1;
			if (!rsLoc->eof) {
				nFirstLocID = AdoFldLong(rsLoc, "ID", -1);
			}

			//set it to -1 because we don't want to default it to anything
			//nLocID = -1;

			// (a.walling 2007-10-10 16:45) - PLID 27726 - Not just yet. Let's try to set it to the most common
			// managed location for this particular contact type.
			CString strContactTable;

			if (m_btnMain.GetCheck()) {
				strContactTable = "ProvidersT";
			} else if (m_btnRef.GetCheck()) {
				strContactTable = "ReferringPhysT";
			} else if (m_btnEmp.GetCheck()) {
				strContactTable = "UsersT";
			} else if (m_btnSup.GetCheck()) {
				strContactTable = "SupplierT";
			} else if (m_btnOther.GetCheck()) {
				strContactTable = "ContactsT";
			}

			if (strContactTable.GetLength()) {

				// (a.walling 2007-10-11 13:52) - PLID 27726 - Included check for Active
				CString strDisambigSql;
				strDisambigSql.Format(
					"SELECT LocationsT.ID, "
						"COUNT(*) AS Count "
					"FROM PersonT "
					"INNER JOIN %s "
					"ON PersonID = PersonT.ID "
					"INNER JOIN LocationsT "
					"ON PersonT.Location = LocationsT.ID "
					"WHERE LocationsT.Managed = 1 "
					"AND LocationsT.Active = 1 "
					"GROUP BY LocationsT.ID "
					"ORDER BY COUNT(*) DESC", strContactTable);

				_RecordsetPtr rs = CreateRecordsetStd(strDisambigSql);

				if (!rs->eof) {
					// we have some rows, so the top row should be the most common managed location for this contact type.
					nLocID = AdoFldLong(rs, "ID", nFirstLocID);
				} else {
					nLocID = nFirstLocID;
				}
			} else {
				nLocID = nFirstLocID;
			}
		}*/

		//(e.lally 2007-02-27) PLID 24886 - Moving this down next to the batch execution, past the modal dlgs
			//to minimize the risk of someone else stealing the next person ID before we commit our queries.
		//nNewID = NewNumber("PersonT", "ID");
		//TES 5/29/03: Look up their default dr. prefix (since it's not hard-coded anymore).
		CString strPrefix = "NULL";
		long nPrefixID;
		if(m_btnMain.GetCheck() || m_btnRef.GetCheck()) {
			nPrefixID = GetRemotePropertyInt("DefaultDrPrefix", 5, 0, "<None>", true);
			strPrefix.Format("%li", nPrefixID);
			if(!ReturnsRecords("SELECT ID FROM PrefixT WHERE ID = %s", strPrefix)) {
				strPrefix = "NULL";
			}
		}

		//TES 9/8/2008 - PLID 27727 - Use the logged-in location.
		//(c.copits 2011-09-15) PLID 35359 - Add Fax and Email to new Ref. Physician contact screen.
		AddStatementToSqlBatch(strSaveString, "INSERT INTO [PersonT] (ID, Gender, HomePhone, WorkPhone, Fax, Email, "
			"Extension, First, Middle, Last, Address1, Address2, City, State, "
			"Zip, Note, Company, UserID, Location, PrefixID) "
			"SELECT  @@NewPersonID@@, %li, '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', '%s', "
			"'%s', '%s', '%s', %li, %li, %s", 
			nGender, _Q(home), _Q(work), _Q(fax), _Q(email), _Q(ext), _Q(first), _Q(middle), 
			_Q(last), _Q(add1), _Q(add2), _Q(city), _Q(state), _Q(zip), _Q(memo), _Q(company), GetCurrentUserID(), GetCurrentLocationID(), strPrefix);
	

		if(m_btnMain.GetCheck() == 1) { //main phys
			// (j.jones 2006-12-01 10:09) - PLID 22110 - changed to support ClaimProviderID
			//TES 9/8/2008 - PLID 27727 - Dropped DefLocationID
			// (z.manning 2008-12-05 09:15) - PLID 28277 - Added NPI
			// (j.jones 2009-06-26 09:25) - PLID 34292 - ensured the OHIP Specialty is filled
			//(e.lally 2012-04-06) PLID 48264 - Added default EMR MU progress settings
			// (s.tullis 2015-02-23 13:43) - PLID 50955 - Remove any default taxonomy codes that we fill in for providers. Now that we support multiple specialities this can lead to billing errors.
			AddStatementToSqlBatch(strSaveString, "INSERT INTO ProvidersT (PersonID, TaxonomyCode, ClaimProviderID, NPI, OHIPSpecialty) VALUES (@@NewPersonID@@, '', @@NewPersonID@@, '%s', '08')", _Q(strNpi));
			AddStatementToSqlBatch(strSaveString, "INSERT INTO InsuranceAcceptedT (InsuranceCoID, ProviderID, Accepted) SELECT PersonID, @@NewPersonID@@ AS ProviderID, %li AS Accepted FROM InsuranceCoT",GetRemotePropertyInt("DefaultInsAcceptAssignment",1,0,"<None>",TRUE) == 1 ? 1 : 0);
			AddStatementToSqlBatch(strSaveString, "INSERT INTO ProviderMUMeasureOptionT (ProviderID, DateOptionNum, StartDate, ExcludeSecondaryProv) VALUES(@@NewPersonID@@, 4, NULL, 0) ");
			for(int nMeasure = 18; nMeasure <= 36; nMeasure ++){
				AddStatementToSqlBatch(strSaveString, "INSERT INTO ProviderMUMeasureSelectionT (ProviderID, MeasureNum, Selected) VALUES(@@NewPersonID@@, %li, 1)", nMeasure);
			}
		}
		else if (m_btnRef.GetCheck() == 1) {	//refer phys
			// (z.manning 2008-12-05 09:15) - PLID 28277 - Added NPI
			AddStatementToSqlBatch(strSaveString, "INSERT INTO [ReferringPhyST] (PersonID, NPI) VALUES (@@NewPersonID@@, '%s')", _Q(strNpi));
		}
		else if (m_btnSup.GetCheck() == 1)	//supplier
		{	
			AddStatementToSqlBatch(strSaveString, "INSERT INTO [SupplierT] (PersonID) SELECT @@NewPersonID@@");
		}
		else if (m_btnOther.GetCheck() == 1){	//other
			
			AddStatementToSqlBatch(strSaveString, "INSERT INTO [ContactsT] (PersonID) SELECT @@NewPersonID@@");
		}

		//(e.lally 2007-02-27) PLID 24886 - To help avoid PK violations, we moved the call to get the new
			//Person ID down here to just before the batch execution. We will replace our new ID placeholder string
			//with the nNewID once it has been calculated.
		nNewID = NewNumber("PersonT", "ID");
		CString strNewPersonID;
		strNewPersonID.Format("%li", nNewID);

		// (r.farnworth 2015-12-29 11:12) - PLID 67719 - Need to declare this now since everything with it is done in seperate if statements
		NexTech_Accessor::_UserPropertiesPtr pUserProperties(__uuidof(NexTech_Accessor::UserProperties));

		if (m_btnEmp.GetCheck() == 1)	//employee
		{
			// (j.gruber 2010-04-14 13:52) - PLID 38186 - don't show the configure groups button
			dlg.m_bShowConfigureGroups = FALSE;
			if(dlg.DoModal() == IDOK)
			{
				// (r.farnworth 2015-12-29 11:12) - PLID 67719 - Change Practice to call our new ChangeContactInformation API method instead of calling SQL from C++ code.
				NexTech_Accessor::_ChangePasswordPtr pChangePassword(__uuidof(NexTech_Accessor::ChangePassword));
				NexTech_Accessor::_UserPtr pUser(__uuidof(NexTech_Accessor::User));

				CString strLocID;
				strLocID.Format("%li", GetCurrentLocationID());
				pUser->PutID(AsBstr(strNewPersonID));
				pUser->Putusername(AsBstr(dlg.m_name));
				pUser->PutlocationID(AsBstr(strLocID));

				pChangePassword->User = pUser;
				pChangePassword->PutNewPassword(AsBstr(dlg.m_pass));
				pChangePassword->PutCurrentPassword(AsBstr(dlg.m_pass));

				pUserProperties->ChangePassword = pChangePassword;
				pUserProperties->PutReason(NexTech_Accessor::UserPropertyChangeReason::UserPropertyChangeReason_NewContact);
				pUserProperties->PutSavePassword((VARIANT_BOOL)dlg.m_bMemory);
				pUserProperties->PutAdministrator((VARIANT_BOOL)dlg.m_bAdministrator);
				pUserProperties->PutAutoLogoff((VARIANT_BOOL)dlg.m_bInactivity);
				pUserProperties->PutAutoLogoffDuration(dlg.m_nInactivityMinutes);
				pUserProperties->PutPasswordExpires((VARIANT_BOOL)dlg.m_bExpires);
				pUserProperties->PutPasswordExpireDays(dlg.m_nPwExpireDays);
				COleDateTime dtNow = COleDateTime::GetCurrentTime();
				dtNow.SetDate(dtNow.GetYear(), dtNow.GetMonth(), dtNow.GetDay());
				pUserProperties->PutPasswordPivotDate(dtNow);
				pUserProperties->PutPasswordExpireNextLogin((VARIANT_BOOL)(dlg.m_bPasswordExpiresNextLogin ? 1 : 0));
				pUserProperties->PutAllLocations((VARIANT_BOOL)dlg.m_bAllLocations);
				pUserProperties->PutIsInternal((VARIANT_BOOL)IsNexTechInternal());
				pUserProperties->PutproviderID(dlg.m_nNewProviderID);

			}
			else
			{
				return false;
			}
		}

		strSaveString.Replace("@@NewPersonID@@", strNewPersonID);
		ExecuteSqlBatch(strSaveString);

		// (r.farnworth 2015-12-30 15:31) - PLID 67719 - Make the call to the API
		if (m_btnEmp.GetCheck() == 1)	//employee
		{
			GetAPI()->UpdateUserProperties(GetAPISubkey(), GetAPILoginToken(), pUserProperties);
		}

		// (v.maida 2014-12-23 12:19) - PLID 64472 - Export new referring physicians, if that option has been selected in the current HL7 settings.
		if (m_btnRef.GetCheck() == BST_CHECKED && IsDlgButtonChecked(IDC_SEND_REF_PHYS_TO_HL7)){ // make sure that both a new referring physician is being saved, and that they want to export that ref phys.
			AddOrUpdateRefPhysInHL7(nNewID, true);
		}
		
/*  Description of a deadlocking problem that occurs when not connected to NxServer on 11/19 (fixed at that point)

Don, the problem is in the exe, not the datalist.
Did you look at \\luke\reference\development\testing\trans1.exe the other day?
Try opening up two instances of trans1.exe:
  - Click "Create Conn", "Begin Trans", and "Write Data" on ONE
  - Click "Create Conn" on THE OTHER
  - Move them on the screen in a way to make both visible at the same time
  - Click "Query Data" on THE OTHER and see how it hangs (the timeouts are 
    hard-coded for like 7 seconds or something)
  - While THE OTHER is hanging click "End Trans" on THE ONE and watch how 
    THE OTHER immediately stops hanging!

That's what's happening here.  The RefreshTable gets processed, starting 
it's requery because that's its job and then the requery just waits, hoping
that we will close our transaction before it times out.  BUT, since we're 
not connected to NxServer, the RefreshTable doesn't post a message, it
just processes the posted message immediately.  In other words, we are 
waiting for the NetUtils::Providers message to be processed before we close
our transaction, but the NetUtils::Providers message handler is waiting for
us to close our transaction before it returns...so there ya go, textbook
deadlock scenario.

Of course it's not exactly that simple.

The reason it's not happening on the Requery itself is that the Requery 
actually does very little on its own.  It just spawns another thread (the 
one that will be doing the querying) and returns immediately.  But anything 
that sets the selection in the datalist HAS to wait for the querying thread 
to take a breath before it commits the selection change (we don't want the 
querying thread to be writing some bits of a variable's memory at the same 
time that something else is reading from some other bits in that same 
memory, we'd end up reading garbage...).  But since the querying thread is
sitting there waiting for the sql server to respond it never takes a breath
(until it times out...hehe) and there ya go.

BUT, it still comes down to the same fact.  You can't ask SQL to read/write 
anything to/from a database in one connection if that database is in the 
middle of a transaction started from another connection.  The solution is
to close this transaction prior to calling any functions that could 
conceivably block on the datalist.
*/

		//now for auditing and network code

		if(m_btnMain.GetCheck() == 1) { //main phys

			AuditItem = aeiProviderCreated;
			
			CClient::RefreshTable(NetUtils::Providers, nNewID);
		}
		else if (m_btnRef.GetCheck() == 1) {	//refer phys

			AuditItem = aeiRefPhysCreated;
			
			CClient::RefreshTable(NetUtils::RefPhys, nNewID);
		}
		else if (m_btnEmp.GetCheck() == 1)	//employee
		{
			//(e.lally 2007-02-27) PLID 24886 - This copy permissions was here before this item, and
				//it is important to stay here now that we don't pass the new person ID into this dlg.
			// (j.gruber 2010-04-14 13:28) - PLID 38186 - switch to configuring groups - if they aren't already an admin
			//CCopyPermissionsDlg::CopyPermissions(&dlg.m_adwImportPermissionsFrom, nNewID);
			
			if (!dlg.m_bAdministrator) {
				if (CheckCurrentUserPermissions(bioEditOtherUserPermissions, sptWrite, 0, 0, TRUE)) {
					if (IDYES == MsgBox(MB_YESNO, "Would you like to configure permission groups for this user?")) {
						CConfigurePermissionGroupsDlg dlg(TRUE, nNewID, this);
						dlg.DoModal();
					}
				}
			}

			CClient::RefreshTable(NetUtils::Coordinators, nNewID);
		}
		else if (m_btnSup.GetCheck() == 1)	//supplier
		{
			AuditItem = aeiSupplierCreated;

			CClient::RefreshTable(NetUtils::Suppliers, nNewID);
		}
		else if (m_btnOther.GetCheck() == 1){	//other

			AuditItem = aeiOtherCreated;

			CClient::RefreshTable(NetUtils::ContactsT, nNewID);
		}

		//In any case, the custom contact list should change.
		CClient::RefreshTable(NetUtils::CustomContacts, nNewID);

		//audit the event
		long nAuditID = -1;
		nAuditID = BeginNewAuditEvent();
		// (r.farnworth 2016-01-06 12:04) - PLID 67704 - We will audit in the API for users
		if(nAuditID != -1 && m_btnEmp.GetCheck() != 1) {
			CString strFullName;
			strFullName.Format("%s, %s %s - %s", last, first, middle, company);
			AuditEvent(-1, strFullName, nAuditID, AuditItem, nNewID, "0", "1", aepMedium, aetCreated);
		}

		//Update outlook
		PPCModifyContact(nNewID);

		*m_pID = nNewID;	

		return true;

	}NxCatchAll("Error in CNewContact::Save()");

	//if we got here, we failed
	return false;

}

BOOL CNewContact::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (c.haag 2008-04-22 17:53) - PLID 29756 - NxIconify the buttons
	m_btnSaveEdit.AutoSet(NXB_OK);
	m_btnSaveResume.AutoSet(NXB_OK);
	m_btnCancel.AutoSet(NXB_CANCEL);
	
	GetDlgItem(IDC_EMPLOYER_BOX)->SetFocus();
	m_bFormatPhoneNums = GetRemotePropertyInt("FormatPhoneNums", 1, 0, "<None>", true); 
	m_strPhoneFormat = GetRemotePropertyText("PhoneFormatString", "(###) ###-####", 0, "<None>", true);

	//(e.lally 2005-05-23) - Allow the Save and Edit button to be disabled
	if(m_bSaveEditEnable)
	{
		//enable the save and edit further button
		m_btnSaveEdit.EnableWindow(TRUE);
	}
	else
	{
		//disable the save and edit further button
		m_btnSaveEdit.EnableWindow(FALSE);

	}

	// set the check for contact type depending on the value for the default type
	// if there's no type, then default to other
	switch (m_nDefaultType)
	{
	case 1:
		m_btnMain.SetCheck(1);
		break;
	case 2:
		m_btnRef.SetCheck(1);
		break;
	case 3:
		m_btnSup.SetCheck(1);
		break;
	case 4:
		m_btnOther.SetCheck(1);
		break;
	case 5:
		m_btnEmp.SetCheck(1);
		break;
	default:
		m_btnOther.SetCheck(1);
		break;
	}

	UpdateTypeSpecificFields(); // (z.manning 2008-12-05 09:26) - PLID 28277

	// (j.gruber 2009-10-06 17:02) - PLID 35826 - reset here in case they changed the preference
	m_bLookupByCity = GetRemotePropertyInt("LookupZipStateByCity", 0, 0, "<None>");

	if (m_bLookupByCity) {
		ChangeZOrder(IDC_ZIP_BOX, IDC_STATE_BOX);
	} else {
		ChangeZOrder(IDC_ZIP_BOX, IDC_ADDRESS2_BOX);
	}
	
	//(c.copits 2011-09-15) PLID 35359 - Add Fax and Email to new Ref. Physician contact screen.
	m_nxeditEmailBox.SetLimitText(50);
	m_nxeditFaxBox.SetLimitText(20);

	return FALSE;
}

int CNewContact::DoModal(long *id, long nDefType /* = 0 */)
{
	m_pID = id;
	// need to see if we have a default contact type
	m_nDefaultType = nDefType;
	return CNxDialog::DoModal();
}

BOOL CNewContact::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	int nID;
	CString str;

	switch(HIWORD(wParam))
	{	case EN_CHANGE:
			switch (nID = LOWORD(wParam))
			{
				case IDC_MIDDLE_NAME_BOX: 
					// (c.haag 2006-08-02 11:40) - PLID 21740 - We now check for auto-capitalization
					// for middle name boxes
					if (GetRemotePropertyInt("AutoCapitalizeMiddleInitials", 1, 0, "<None>", true)) {
						Capitalize(nID);
					}
					break;
				case IDC_FIRST_NAME_BOX: 
				case IDC_LAST_NAME_BOX: 
				case IDC_ADDRESS1_BOX: 
				case IDC_ADDRESS2_BOX: 
				case IDC_CITY_BOX: 
				case IDC_STATE_BOX: 
				case IDC_EMPLOYER_BOX: 
					Capitalize(nID);
					break;
				case IDC_ZIP_BOX:
					// (d.moore 2007-04-23 12:11) - PLID 23118 - 
					//  Capitalize letters in the zip code as they are typed in. Canadian postal
					//    codes need to be formatted this way.
					CapitalizeAll(IDC_ZIP_BOX);
					GetDlgItemText(nID, str);
					str.TrimRight();
					break;
				case IDC_HOME_PHONE_BOX: 
				case IDC_WORK_PHONE_BOX: 
				//(c.copits 2011-09-15) PLID 35359 - Add Fax and Email to new Ref. Physician contact screen.
				case IDC_FAX_BOX:
					GetDlgItemText(nID, str);
					str.TrimRight();
					if (str != "") {
						if(m_bFormatPhoneNums) {
							FormatItem (nID, m_strPhoneFormat);
						}
					}
					break;	
				case IDC_EXT_PHONE_BOX: 
					GetDlgItemText(nID, str);
					str.TrimRight();
					if (str != "")
						FormatItem (nID, "nnnnnnn");
					break;
				default:
					break;
			}
			m_changed = true;
			break;
		case EN_KILLFOCUS:
			//(c.copits 2011-09-15) PLID 35359 - Add Fax and Email to new Ref. Physician contact screen.
			if (LOWORD(wParam) == IDC_HOME_PHONE_BOX || LOWORD(wParam) == IDC_WORK_PHONE_BOX || LOWORD(wParam) == IDC_FAX_BOX) {
				//is the member variable empty
				if (!m_strAreaCode.IsEmpty() ) {
					//check to see if that is the only thing that is in the box
					CString strPhone;
					GetDlgItemText(LOWORD(wParam), strPhone);
					if (strPhone == m_strAreaCode) {
						//if they are equal then erase the area code
						SetDlgItemText(LOWORD(wParam), "");
					}
				}
				//set out member variable to blank
				m_strAreaCode = "";
			}
			break;

		case EN_SETFOCUS:
			{
				int nID = LOWORD(wParam);
				//(c.copits 2011-09-15) PLID 35359 - Add Fax and Email to new Ref. Physician contact screen.
				if(nID == IDC_HOME_PHONE_BOX || nID == IDC_WORK_PHONE_BOX || nID == IDC_FAX_BOX) {
					if (ShowAreaCode()) {
						//first check to see if anything is in this box
						CString strPhone;
						GetDlgItemText(nID, strPhone);
						if (! ContainsDigit(strPhone)) {
							// (j.gruber 2009-10-07 17:01) - PLID 35826 - updated for city lookup
							CString strAreaCode, strZip, strCity;
							GetDlgItemText(IDC_ZIP_BOX, strZip);
							GetDlgItemText(IDC_CITY_BOX, strCity);
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
				}
			}
			break;

		default:
			break;
	}	
	return CNxDialog::OnCommand(wParam, lParam);
}

void CNewContact::OnSaveAndEdit() 
{
	if (Save())
	{
		//if we are currently filtering a group, and this new contact is not in that group type,
		//we must set the type to all
		int nStatus = GetMainFrame()->m_contactToolBar.GetActiveContactStatus();

		if ((nStatus != 1 && m_btnRef.GetCheck()) || (nStatus != 2 && m_btnMain.GetCheck()) || (nStatus != 4 && m_btnEmp.GetCheck()) || (nStatus != 8 && m_btnSup.GetCheck()))
		{	//ref phys								//main phys								//user									//supplier
			GetMainFrame()->m_contactToolBar.m_main.SetCheck(false);
			GetMainFrame()->m_contactToolBar.m_supplier.SetCheck(false);
			GetMainFrame()->m_contactToolBar.m_other.SetCheck(false);
			GetMainFrame()->m_contactToolBar.m_employee.SetCheck(false);
			GetMainFrame()->m_contactToolBar.m_referring.SetCheck(false);
			GetMainFrame()->m_contactToolBar.m_all.SetCheck(true);
		}
		
		EndDialog(2);
	}	
}

void CNewContact::OnSaveAndResume() 
{
	if (Save())
		EndDialog(1);	
}

void CNewContact::OnCancel() 
{
	EndDialog(0);	
}

//DRT 6/2/2008 - PLID 30230 - Added OnOK handler to keep behavior the same as pre-NxDialog changes
void CNewContact::OnOK()
{
	//Eat the message
}

// (z.manning 2008-12-05 08:45) - PLID 28277
void CNewContact::OnBnClickedMainBtn()
{
	try
	{
		UpdateTypeSpecificFields();

	}NxCatchAll("CNewContact::OnBnClickedMainBtn");
}

// (z.manning 2008-12-05 08:45) - PLID 28277
void CNewContact::OnBnClickedEmployeeBtn()
{
	try
	{
		UpdateTypeSpecificFields();

	}NxCatchAll("CNewContact::OnBnClickedEmployeeBtn");
}

// (z.manning 2008-12-05 08:45) - PLID 28277
void CNewContact::OnBnClickedRefphysBtn()
{
	try
	{
		UpdateTypeSpecificFields();

	}NxCatchAll("CNewContact::OnBnClickedRefphysBtn");
}

// (z.manning 2008-12-05 08:45) - PLID 28277
void CNewContact::OnBnClickedSupplierBtn()
{
	try
	{
		UpdateTypeSpecificFields();

	}NxCatchAll("CNewContact::OnBnClickedSupplierBtn");
}

// (z.manning 2008-12-05 08:45) - PLID 28277
void CNewContact::OnBnClickedOtherBtn()
{
	try
	{
		UpdateTypeSpecificFields();

	}NxCatchAll("CNewContact::OnBnClickedOtherBtn");
}

// (z.manning 2008-12-04 17:51) - PLID 28277 - Shows/hides fields based on the current contact type
void CNewContact::UpdateTypeSpecificFields()
{
	if(m_btnMain.GetCheck() == BST_CHECKED || m_btnRef.GetCheck() == BST_CHECKED) {
		m_nxstaticNpiLabel.ShowWindow(SW_SHOW);
		m_nxeditNpi.ShowWindow(SW_SHOW);
		if (m_btnRef.GetCheck() == BST_CHECKED) { // (v.maida 2014-12-23 12:19) - PLID 64472 - Additionally, if this is a referring physician contact, the "send to HL7" checkbox needs to be shown.
			GetDlgItem(IDC_SEND_REF_PHYS_TO_HL7)->ShowWindow(SW_SHOW); // show the send to HL7 checkbox
			// check or don't check the send to HL7 checkbox, depending on whether or not the "automatically export new referring physicians" option is set for at least one HL7 group.
			CArray<long, long> arHL7GroupIDs;
			GetHL7SettingsGroupsBySetting("EXPORTNEWREFPHYS", TRUE, arHL7GroupIDs);
			if (!g_pLicense->CheckForLicense(CLicense::lcHL7, CLicense::cflrSilent) || arHL7GroupIDs.GetSize() == 0) {
				GetDlgItem(IDC_SEND_REF_PHYS_TO_HL7)->EnableWindow(FALSE);
			}
			else {
				CheckDlgButton(IDC_SEND_REF_PHYS_TO_HL7, BST_CHECKED);
			}
		}
		else {
			GetDlgItem(IDC_SEND_REF_PHYS_TO_HL7)->ShowWindow(SW_HIDE);
		}
	}
	else {
		m_nxstaticNpiLabel.ShowWindow(SW_HIDE);
		m_nxeditNpi.ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SEND_REF_PHYS_TO_HL7)->ShowWindow(SW_HIDE);
	}
}

//(c.copits 2011-09-22) PLID 45626 - Validate Email Addresses
void CNewContact::OnKillfocusEmailBox()
{
	try {
		
		CString strEmailAddress;
		GetDlgItemText(IDC_EMAIL_BOX, strEmailAddress);
		
		strEmailAddress.TrimLeft();
		strEmailAddress.TrimRight();
		
		// Blank (no) email addresses are OK.
		if (strEmailAddress.IsEmpty()) {
			return;
		}

		if (!IsValidEmailAddress(strEmailAddress)) {
			CString strErrorMessage = "Your email address is invalid. Ensure that it contains:\n\n"
				"- A user name\n"
				"- One @ symbol\n"
				"- A Domain name\n"
				"- No special characters (such as spaces and quotes)\n\n"
				"Example: example@nextech.com";
			MessageBox(strErrorMessage, "Invalid Email Address", MB_ICONWARNING|MB_OK);
			GetDlgItem(IDC_FIRST_NAME_BOX)->SetFocus();
		}

	} NxCatchAll(__FUNCTION__);
}

