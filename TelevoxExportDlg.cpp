// TelevoxExportDlg.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "TelevoxExportDlg.h"
#include "InternationalUtils.h"
#include "TelevoxOutputDlg.h"
#include "TelevoxConfigureFieldsDlg.h"
#include "PatientReminderSenthistoryUtils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CTelevoxExportDlg dialog


CTelevoxExportDlg::CTelevoxExportDlg(CWnd* pParent)
	: CNxDialog(CTelevoxExportDlg::IDD, pParent)
{
	// (b.savon 2014-08-28 11:29) - PLID 62790 - Holds the current status - Default is do nothing
	m_rssPatientReminder = rssNothing;
}


void CTelevoxExportDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTelevoxExportDlg)
	DDX_Control(pDX, IDC_CHECK_LOCATION, m_btnLocation);
	DDX_Control(pDX, IDC_EXCLUDE_PRIVATE, m_btnExcludePriv);
	DDX_Control(pDX, IDC_USE_APPT_PURPOSE, m_btnUsePurpose);
	DDX_Control(pDX, IDC_SEND_EMAIL, m_btnSendEmail);
	DDX_Control(pDX, IDC_DATE_FROM, m_dtFrom);
	DDX_Control(pDX, IDC_DATE_TO, m_dtTo);
	DDX_Control(pDX, IDC_CONFIGURE_TELEVOX_FIELDS, m_btnConfigureFields);
	DDX_Control(pDX, IDC_FAIL_IF_NOT_10_DIGITS, m_btnFailIfNot10Digits);
	DDX_Control(pDX, IDC_NO_CELL_NO_EXPORT, m_btnNoCellNoExport);
	DDX_Control(pDX, IDC_NO_CELL_IF_NO_TEXT_MSG, m_btnNoCellIfNoTextMsg);
	DDX_Control(pDX, IDC_SELECT_TYPE, m_btnSelectType);
	DDX_Control(pDX, IDC_UNSELECT_TYPE, m_btnUnselectType);
	DDX_Control(pDX, IDC_SELECT_RESOURCE, m_btnSelectResource);
	DDX_Control(pDX, IDC_UNSELECT_RESOURCE, m_btnUnselectResource);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTelevoxExportDlg, CNxDialog)
	//{{AFX_MSG_MAP(CTelevoxExportDlg)
	ON_BN_CLICKED(IDC_CHECK_LOCATION, OnCheckLocation)
	ON_BN_CLICKED(IDC_SELECT_TYPE, OnSelectType)
	ON_BN_CLICKED(IDC_UNSELECT_TYPE, OnUnselectType)
	ON_BN_CLICKED(IDC_SELECT_RESOURCE, OnSelectResource)
	ON_BN_CLICKED(IDC_UNSELECT_RESOURCE, OnUnselectResource)
	ON_BN_CLICKED(IDC_GENERATE_PATIENTS, OnGeneratePatients)
	ON_BN_CLICKED(IDC_CONFIGURE_TELEVOX_FIELDS, OnConfigureTelevoxFields)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_RADIO_DO_NOTHING, &CTelevoxExportDlg::OnBnClickedRadioDoNothing)
	ON_BN_CLICKED(IDC_RADIO_ADD_ALL_PATIENTS, &CTelevoxExportDlg::OnBnClickedRadioAddAllPatients)
	ON_BN_CLICKED(IDC_RADIO_ADD_NO_APPTS, &CTelevoxExportDlg::OnBnClickedRadioAddNoAppts)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTelevoxExportDlg message handlers

BOOL CTelevoxExportDlg::OnInitDialog() 
{
	CNxDialog::OnInitDialog();

	// (b.savon 2014-09-02 10:36) - PLID 62791 - Cache dontshow TeleVoxPatientReminder; also do the existing
	g_propManager.CachePropertiesInBulk("TelevoxExportDlg", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'TelevoxEmail' "
		"OR Name = 'TelevoxPurpose' "
		"OR Name = 'TelevoxFailIfNot10Digits' "
		"OR Name = 'TelevoxNoCellNoExport' "
		"OR Name = 'TelevoxNoCellIfNoTextMsg' "
		"OR Name = 'dontshow TeleVoxPatientReminder_AllPatients' "
		"OR Name = 'dontshow TeleVoxPatientReminder_NoFutureAppointments' "
		")",
		_Q(GetCurrentUserName())
	);

	// TODO: Add extra initialization here
	m_pLocations = BindNxDataListCtrl(IDC_LOCATION_FILTER_LIST);
	m_pUnselTypes = BindNxDataListCtrl(IDC_UNSEL_TYPE);
	m_pSelTypes = BindNxDataListCtrl(IDC_SEL_TYPE, false);
	m_pUnselRes = BindNxDataListCtrl(IDC_UNSEL_RESOURCE);
	m_pSelRes = BindNxDataListCtrl(IDC_SEL_RESOURCE, false);

	m_btnConfigureFields.AutoSet(NXB_MODIFY);
	// (z.manning 2008-08-14 10:23) - PLID 29946 - Use icons for the select/unselect buttons
	m_btnSelectType.AutoSet(NXB_RIGHT);
	m_btnUnselectType.AutoSet(NXB_LEFT);
	m_btnSelectResource.AutoSet(NXB_RIGHT);
	m_btnUnselectResource.AutoSet(NXB_LEFT);

	OnCheckLocation();

	//set the default location selection to the current location
	m_pLocations->SetSelByColumn(0, (long)GetCurrentLocationID());

	//set the dates to tomorrow - that seems to be the most likely set of patients
	//that will be generated
	COleDateTimeSpan dtSpan(1, 0, 0, 0);
	COleDateTime dtTomorrow = COleDateTime::GetCurrentTime() + dtSpan;
	m_dtFrom.SetValue(_variant_t(dtTomorrow));
	m_dtTo.SetValue(_variant_t(dtTomorrow));

	//default the exclude to checked
	CheckDlgButton(IDC_EXCLUDE_PRIVATE, TRUE);

	//recall the option on whether the email checkbox is set or not
	UINT nSet = GetRemotePropertyInt("TelevoxEmail", 0, 0, "<None>", false);
	CheckDlgButton(IDC_SEND_EMAIL, nSet);

	//recall the option for purpose as well
	nSet = GetRemotePropertyInt("TelevoxPurpose", 0, 0, "<None>", false);
	CheckDlgButton(IDC_USE_APPT_PURPOSE, nSet);

	// (z.manning 2008-07-10 16:18) - PLID 20543 - Added option to not necessarily fail
	// for non-10 digit phone numbers.
	nSet = GetRemotePropertyInt("TelevoxFailIfNot10Digits", BST_CHECKED, 0, "<None>", false);
	m_btnFailIfNot10Digits.SetCheck(nSet);

	// (z.manning 2008-07-10 17:10) - PLID 20543 - Added option to not export patients without cell
	// phone numbers
	nSet = GetRemotePropertyInt("TelevoxNoCellNoExport", BST_UNCHECKED, 0, "<None>", false);
	m_btnNoCellNoExport.SetCheck(nSet);

	// (z.manning 2008-07-11 13:31) - PLID 30678 - Preference to not export cell number
	// if patient doesn't want text messages.
	nSet = GetRemotePropertyInt("TelevoxNoCellIfNoTextMsg", BST_CHECKED, 0, "<None>", false);
	m_btnNoCellIfNoTextMsg.SetCheck(nSet);

	// (b.savon 2014-08-28 11:47) - PLID 62790 - Check the selection - Default is rssNothing - give func to change in the future
	switch (m_rssPatientReminder)
	{
		case rssNoFutureAppts:
			CheckDlgButton(IDC_RADIO_ADD_NO_APPTS, BST_CHECKED);
			break;
		case rssAllPatients:
			CheckDlgButton(IDC_RADIO_ADD_ALL_PATIENTS, BST_CHECKED);
			break;
		case rssNothing:
			CheckDlgButton(IDC_RADIO_DO_NOTHING, BST_CHECKED);
			break;
	}
	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTelevoxExportDlg::OnOK() 
{
	//CDialog::OnOK();
}

void CTelevoxExportDlg::OnCancel() 
{
	CDialog::OnCancel();
}

void CTelevoxExportDlg::OnCheckLocation() 
{
	if(IsDlgButtonChecked(IDC_CHECK_LOCATION))
		m_pLocations->Enabled = true;
	else
		m_pLocations->Enabled = false;
}

void CTelevoxExportDlg::OnSelectType() 
{
	long nCurSel = m_pUnselTypes->GetCurSel();

	//can't move no row
	if(nCurSel == sriNoRow)
		return;

	//move the row to the selected side
	m_pSelTypes->TakeRow(m_pUnselTypes->GetRow(nCurSel));
}

void CTelevoxExportDlg::OnUnselectType() 
{
	long nCurSel = m_pSelTypes->GetCurSel();

	//can't move no row
	if(nCurSel == sriNoRow)
		return;

	//move the row to the selected side
	m_pUnselTypes->TakeRow(m_pSelTypes->GetRow(nCurSel));
}

void CTelevoxExportDlg::OnSelectResource() 
{
	long nCurSel = m_pUnselRes->GetCurSel();

	//can't move no row
	if(nCurSel == sriNoRow)
		return;

	//move the row to the selected side
	m_pSelRes->TakeRow(m_pUnselRes->GetRow(nCurSel));
}

void CTelevoxExportDlg::OnUnselectResource() 
{
	long nCurSel = m_pSelRes->GetCurSel();

	//can't move no row
	if(nCurSel == sriNoRow)
		return;

	//move the row to the selected side
	m_pUnselRes->TakeRow(m_pSelRes->GetRow(nCurSel));
}

void CTelevoxExportDlg::OnGeneratePatients() 
{
	try {
		//generate the query to output
		CString strLoc;
		CString strTypes;
		CString strRes;
		CString strDate;
		CString strPriv;

		//Location Filter
		if(IsDlgButtonChecked(IDC_CHECK_LOCATION)) {
			long nCurSel = m_pLocations->GetCurSel();
			if(nCurSel == -1) {
				//this should not be possible
				MsgBox("You must select a location before continuing.");
				return;
			}

			//format the string to filter just this location
			strLoc.Format(" AND AppointmentsT.LocationID = %li", VarLong(m_pLocations->GetValue(nCurSel, 0)));
		}

		//Type Filter
		for(int i = 0; i < m_pSelTypes->GetRowCount(); i++) {
			//loop through all selected types
			CString str;
			str.Format(" AppointmentsT.AptTypeID = %li OR ", VarLong(m_pSelTypes->GetValue(i, 0)));

			strTypes += str;
		}

		if(!strTypes.IsEmpty()) {
			//put all we generated into ( )
			strTypes.TrimRight(" OR ");	//the last one 
			strTypes = " AND (" + strTypes + " )";
		}

		//Resource Filter
		for(int j = 0; j < m_pSelRes->GetRowCount(); j++) {
			//loop through all selected types
			CString str;
			str.Format(" ResourceID = %li OR ", VarLong(m_pSelRes->GetValue(j, 0)));

			strRes += str;
		}

		if(!strRes.IsEmpty()) {
			//put all we generated into ( )
			strRes.TrimRight(" OR ");	//the last one 
			strRes = " AND AppointmentsT.ID IN (SELECT AppointmentID FROM AppointmentResourceT WHERE " + strRes + " )";
		}

		//Date Filter
		{
			COleDateTime dtFrom, dtTo;
			dtFrom = COleDateTime(m_dtFrom.GetValue());
			dtTo = COleDateTime(m_dtTo.GetValue());
			COleDateTimeSpan dtSpan(1, 0, 0, 0);
			dtTo += dtSpan;	//add 1 day to the end for filtering purposes

			//format dates for our filter
			// (j.gruber 2008-07-30 12:36) - PLID 30891 - fixed for international compliance
			strDate.Format(" AND AppointmentsT.Date >= '%s' AND AppointmentsT.Date < '%s' ", FormatDateTimeForSql(dtFrom, dtoDate), 
				FormatDateTimeForSql(dtTo, dtoDate));
		}

		//Privacy Filter
		// (j.gruber 2006-12-18 14:56) - PLID 21328 - change this to be the preferred contact also
		// (d.thompson 2010-07-23) - PLID 39800 - Added PreferredContact 7 - textmessage.  This requires BOTH Cell privacy and Text privacy
		// (j.gruber 2011-07-15 15:48) - PLID 37548 - added email
		//	are off.
		if(IsDlgButtonChecked(IDC_EXCLUDE_PRIVATE)) {
			//strPriv.Format(" AND PersonT.PrivHome = 0 ");
			strPriv.Format (" AND 0 =  "
				"  CASE WHEN PatientsT.PreferredContact = 1 THEN CASE WHEN PersonT.PrivHome = 0 THEN 0 ELSE 1 END ELSE  "
				" CASE WHEN PatientsT.PreferredContact = 2 THEN CASE WHEN PersonT.PrivWork = 0 THEN 0 ELSE 1 END ELSE  "
				" CASE WHEN PatientsT.PreferredContact = 3 THEN CASE WHEN PersonT.PrivCell = 0 THEN 0 ELSE 1 END ELSE  "
				" CASE WHEN PatientsT.PreferredContact = 4 THEN CASE WHEN PersonT.PrivPager = 0 THEN 0 ELSE 1 END ELSE  "
				" CASE WHEN PatientsT.PreferredContact = 5 THEN CASE WHEN PersonT.PrivOther = 0 THEN 0 ELSE 1 END ELSE  "
				" CASE WHEN PatientsT.PreferredContact = 6 THEN CASE WHEN PersonT.PrivEmail = 0 THEN 0 ELSE 1 END ELSE  "
				" CASE WHEN PatientsT.PreferredContact = 7 THEN CASE WHEN PersonT.TextMessage = 0 AND PersonT.PrivCell = 0 THEN 0 ELSE 1 END ELSE  "
				" CASE WHEN PersonT.PrivHome = 0 THEN 0 ELSE 1 END END END END END END END END"); 
		}

		//DRT 7/19/2004 - PLID 13532 - Use purpose as notes?
		CString strNoteField = "AppointmentsT.Notes";
		if(IsDlgButtonChecked(IDC_USE_APPT_PURPOSE)) {
			strNoteField = "dbo.GetPurposeString(AppointmentsT.ID) AS Notes";
		}

		CString sql;
		//PLID 21328 - changed the phone number to be the preferred contact number if they have one, otherwise use the home phone number
		// (z.manning 2008-07-08 14:52) - PLID 20543 - Added cell phone
		// (z.manning 2008-07-11 11:44) - PLID 30678 - Added TextMessage
		// (d.thompson 2010-07-23) - PLID 39800 - Added PreferredContact 7 (textmessage - export cell phone).  This does not affect the
		//	cell phone field.
		// (j.gruber 2011-07-15 15:49) - PLID 37548 - added PreferredContact email
		// (a.wilson 2013-02-25 16:19) - PLID 54637 - added prefcontact and prefcontactvalue to fix removing '-' from emails.
		// (b.savon 2014-08-28 13:37) - PLID 62790 - Added AppointmentsT.PatientID
		sql.Format("SELECT  "
				"/*Required Fields*/ "
				"PersonT.Last, PersonT.First, PatientsT.PreferredContact, "
				"CASE WHEN PatientsT.PreferredContact = 1 THEN PersonT.HomePhone ELSE "
				"CASE WHEN PatientsT.PreferredContact = 2 THEN PersonT.WorkPhone ELSE "
				"CASE WHEN PatientsT.PreferredContact = 3 THEN PersonT.CellPhone ELSE "
				"CASE WHEN PatientsT.PreferredContact = 4 THEN PersonT.Pager ELSE "
				"CASE WHEN PatientsT.PreferredContact = 5 THEN PersonT.OtherPhone ELSE "
				"CASE WHEN PatientsT.PreferredContact = 6 THEN PersonT.Email ELSE "
				"CASE WHEN PatientsT.PreferredContact = 7 THEN PersonT.CellPhone ELSE "
				"PersonT.HomePhone END END END END END END END AS PreferredContactValue, "
				"AppointmentsT.Date, AppointmentsT.StartTime, "
				"AppointmentsT.PatientID, "
				"/*Optional But strongly recommended fields*/ "
				"PatientsT.UserDefinedID, "
				"/*Optional fields*/ "
				"dbo.GetResourceString(AppointmentsT.ID) AS Resource, AptTypeT.Name AS TypeName, "
				"LocationsT.Name AS LocName, "
				"/*Extras*/ "
				"PersonT.Address1, PersonT.City, PersonT.State, PersonT.Zip, PersonT.WorkPhone, %s, PersonT.Email, "
				"CASE WHEN COALESCE(PersonT.PrivCell, 0) = 1 THEN '' ELSE PersonT.CellPhone END AS CellPhone, "
				"PersonT.TextMessage "
				// (z.manning 2008-07-10 16:48) - PLID 20543 - If you add any more fields here then make sure
				// you also handle them in TelevoxFieldsT and TelevoxOutputDlg.
				"FROM AppointmentsT "
				"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
				"LEFT JOIN PatientsT ON PersonT.ID = PatientsT.PersonID "
				"LEFT JOIN AptTypeT ON AppointmentsT.AptTypeID = AptTypeT.ID "
				"LEFT JOIN LocationsT ON AppointmentsT.LocationID = Locationst.ID "
				"WHERE AppointmentsT.Status <> 4 AND AppointmentsT.PatientID > 0 "
				"%s %s %s %s %s", strNoteField, strLoc, strTypes, strRes, strDate, strPriv);		//All filters are blank if not used, or filled with an appropriate clause otherwise

		//now we pass this on to the output dialog
		CTelevoxOutputDlg dlg(this);
		dlg.SetOutputSql(sql);
		dlg.IncludeEmail(IsDlgButtonChecked(IDC_SEND_EMAIL));
		dlg.m_bFailIfNot10Digits = m_btnFailIfNot10Digits.GetCheck() == BST_CHECKED;
		dlg.m_bNoCellNoExport = m_btnNoCellNoExport.GetCheck() == BST_CHECKED;
		// (z.manning 2008-07-11 13:34) - PLID 30678 - Should we be ingnoring cell phone for those who don't want texts?
		dlg.m_bNoCellIfNoTextMsg = m_btnNoCellIfNoTextMsg.GetCheck() == BST_CHECKED;
		int res = dlg.DoModal();

		if(res == IDOK) {
			//we need to save a few settings
			SetRemotePropertyInt("TelevoxEmail", IsDlgButtonChecked(IDC_SEND_EMAIL), 0, "<None>");	//save the state of the email checkbox
			SetRemotePropertyInt("TelevoxPurpose", IsDlgButtonChecked(IDC_USE_APPT_PURPOSE), 0, "<None>");	//save state of override note w/ purpose checkbox
			// (z.manning 2008-07-10 16:26) - PLID 20543 - Added preference to fail if not 10 digits.
			SetRemotePropertyInt("TelevoxFailIfNot10Digits", m_btnFailIfNot10Digits.GetCheck(), 0, "<None>");
			SetRemotePropertyInt("TelevoxNoCellNoExport", m_btnNoCellNoExport.GetCheck(), 0, "<None>");
			// (z.manning 2008-07-11 13:33) - PLID 30678 - Save the no cell if not text message option
			SetRemotePropertyInt("TelevoxNoCellIfNoTextMsg", m_btnNoCellIfNoTextMsg.GetCheck(), 0, "<None>");

			// (b.savon 2014-08-28 13:30) - PLID 62790 - Queue the reminder!
			CreatePatientReminder(sql);

			CDialog::OnOK();
		}

	} NxCatchAll("Error generating patient list.");
}

BEGIN_EVENTSINK_MAP(CTelevoxExportDlg, CNxDialog)
    //{{AFX_EVENTSINK_MAP(CTelevoxExportDlg)
	ON_EVENT(CTelevoxExportDlg, IDC_LOCATION_FILTER_LIST, 1 /* SelChanging */, OnSelChangingLocationFilterList, VTS_PI4)
	ON_EVENT(CTelevoxExportDlg, IDC_UNSEL_TYPE, 3 /* DblClickCell */, OnDblClickCellUnselType, VTS_I4 VTS_I2)
	ON_EVENT(CTelevoxExportDlg, IDC_UNSEL_RESOURCE, 3 /* DblClickCell */, OnDblClickCellUnselResource, VTS_I4 VTS_I2)
	ON_EVENT(CTelevoxExportDlg, IDC_SEL_TYPE, 3 /* DblClickCell */, OnDblClickCellSelType, VTS_I4 VTS_I2)
	ON_EVENT(CTelevoxExportDlg, IDC_SEL_RESOURCE, 3 /* DblClickCell */, OnDblClickCellSelResource, VTS_I4 VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

void CTelevoxExportDlg::OnSelChangingLocationFilterList(long FAR* nNewSel) 
{
	//if they try to select nothing, set the selection to the first row
	if(*nNewSel == -1)
		*nNewSel = 0;
}

void CTelevoxExportDlg::OnDblClickCellUnselType(long nRowIndex, short nColIndex) 
{
	//let the button function do everything, we're just working with the current selection
	OnSelectType();
}

void CTelevoxExportDlg::OnDblClickCellUnselResource(long nRowIndex, short nColIndex) 
{
	//let the button function do everything, we're just working with the current selection
	OnSelectResource();
}

void CTelevoxExportDlg::OnDblClickCellSelType(long nRowIndex, short nColIndex) 
{
	//let the button function do everything, we're just working with the current selection
	OnUnselectType();
}

void CTelevoxExportDlg::OnDblClickCellSelResource(long nRowIndex, short nColIndex) 
{
	//let the button function do everything, we're just working with the current selection
	OnUnselectResource();
}

void CTelevoxExportDlg::OnConfigureTelevoxFields()
{
	try
	{
		// (z.manning 2008-07-10 16:52) - PLID 20543 - Before we open this dialog warn them that
		// changing something here could mess up their TeleVox export.
		int nResult = MessageBox("Warning!\r\n\r\nIf you are an existing TeleVox client and you make changes to which "
			"columns you export or their order then your TeleVox export file may not work anymore. It is "
			"strongly recommended you contact TeleVox before making any changes here.\r\n\r\n"
			"Do you want to continue?", NULL, MB_YESNO|MB_ICONWARNING);
		if(nResult != IDYES) {
			return;
		}

		CTelevoxConfigureFieldsDlg dlg(this);
		dlg.DoModal();

	}NxCatchAll("CTelevoxExportDlg::OnConfigureTelevoxFields");
}

// (b.savon 2014-08-28 13:10) - PLID 62790 - User clicked Do Nothing
void CTelevoxExportDlg::OnBnClickedRadioDoNothing()
{
	try{
		m_rssPatientReminder = rssNothing;
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-08-28 13:10) - PLID 62790 - User clicked All Patients
void CTelevoxExportDlg::OnBnClickedRadioAddAllPatients()
{
	try{
		// (b.savon 2014-09-02 10:27) - PLID 62791 - Dont Show for All Patients Radio
		DontShowMe(this, "TeleVoxPatientReminder_AllPatients");
		m_rssPatientReminder = rssAllPatients;
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-08-28 13:10) - PLID 62790 - User clicked No Future Appts Only
void CTelevoxExportDlg::OnBnClickedRadioAddNoAppts()
{
	try{
		// (b.savon 2014-09-02 10:27) - PLID 62791 - Dont Show for No Future Appointments Radio
		DontShowMe(this, "TeleVoxPatientReminder_NoFutureAppointments");
		m_rssPatientReminder = rssNoFutureAppts;
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-08-28 13:10) - PLID 62790 - Create the patient reminders
void CTelevoxExportDlg::CreatePatientReminder(CString sql)
{
	try{

		CSqlFragment sqlPatientReminder = CSqlFragment(
			R"(
				SELECT	DISTINCT ExportQ.PatientID,
						{INT} AS UserID,				
						{INT} AS Method,				
						GETDATE() AS ReminderSentDate
				FROM
				(
					{SQL}
				) AS ExportQ
			)",
			GetCurrentUserID(),
			srhTeleVox,
			CSqlFragment(sql)
		);

		switch (m_rssPatientReminder)
		{
			case rssNoFutureAppts:
			{
				sqlPatientReminder += CSqlFragment(
					R"(
						LEFT JOIN 
						(
							SELECT PatientID FROM AppointmentsT WHERE AppointmentsT.Status <> 4 AND AppointmentsT.Date > (CONVERT(DATETIME, FLOOR(CONVERT(FLOAT, GETDATE())))) AND AppointmentsT.PatientID > 0
						) AS FutureQ ON ExportQ.PatientID = FutureQ.PatientID
						WHERE FutureQ.PatientID IS NULL
					)"
					);
			}
			break;
			case rssAllPatients:
			{
				sqlPatientReminder += CSqlFragment(
					R"(
						WHERE ExportQ.PatientID > 0
					)"
				);
			}
			break;
			case rssNothing:
			default:
			{
				// Eat it
			}
			break;
		}

		if (m_rssPatientReminder != rssNothing){
			CommitPatientReminders(sqlPatientReminder);
		}
		
	}NxCatchAll(__FUNCTION__);
}

// (b.savon 2014-08-28 14:29) - PLID 62790 - Commit
void CTelevoxExportDlg::CommitPatientReminders(const CSqlFragment &sql)
{
	try{
		CSqlFragment sqlInsertPatientReminder = CSqlFragment(
			R"(
				INSERT INTO PatientRemindersSentT(PatientID, UserID, ReminderMethod, ReminderDate)
				{SQL}
			)",
			sql
		);

		ExecuteParamSql(sqlInsertPatientReminder);

	}NxCatchAll(__FUNCTION__);
}