// PatientSummaryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "PatientSummaryDlg.h"
#include "DateTimeUtils.h"
#include "InternationalUtils.h"
#include "GlobalFinancialUtils.h"
#include "DontShowDlg.h"
#include "DocumentOpener.h"
#include "GlobalLabUtils.h"
#include "PatientSummaryConfigDlg.h"
#include "SharedScheduleUtils.h"

// (a.walling 2010-11-26 13:08) - PLID 40444 - Updated module tab enums and related code

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (j.jones 2008-07-08 09:14) - PLID 24624 - created

/////////////////////////////////////////////////////////////////////////////
// CPatientSummaryDlg dialog

using namespace NXDATALIST2Lib;
using namespace ADODB;

#define TIMER_LOAD_SUMMARY_LIST 1

enum SummaryListColumns {

	slcDate = 0,
	slcCategory,
	slcDescription,
};

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
enum DemoListColumns {
	dlcLeft = 0,
	dlcLeftLink,
	dlcRight,
	dlcRightLink,
};
	

CPatientSummaryDlg::CPatientSummaryDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPatientSummaryDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CPatientSummaryDlg)
		m_nPatientID = -1;
	//}}AFX_DATA_INIT
}


void CPatientSummaryDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPatientSummaryDlg)
	DDX_Control(pDX, IDC_PATIENT_SUMMARY_BKG, m_bkg);
	DDX_Control(pDX, IDC_BTN_SUMMARY_HELP, m_btnHelp);
	DDX_Control(pDX, IDC_BTN_SUMMARY_CLOSE, m_btnClose);
	DDX_Control(pDX, IDC_PATIENT_SUMMARY_STATUS_LABEL, m_nxstaticStatusLabel);
	DDX_Control(pDX, IDC_BTN_SUMMARY_CONFIGURE, m_btnConfigure);	
	//DDX_Control(pDX, IDC_EDIT_LAST_APPOINTMENT_INFO, m_editLastApptInfo);
	//DDX_Control(pDX, IDC_EDIT_LAST_DIAG_CODES, m_editLastDiagCodes);
	//DDX_Control(pDX, IDC_EDIT_PATIENT_BALANCE, m_editPatientBalance);
	//DDX_Control(pDX, IDC_EDIT_INSURANCE_BALANCE, m_editInsuranceBalance);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPatientSummaryDlg, CNxDialog)
	//{{AFX_MSG_MAP(CPatientSummaryDlg)
	ON_BN_CLICKED(IDC_BTN_SUMMARY_HELP, OnBtnSummaryHelp)
	ON_BN_CLICKED(IDC_BTN_SUMMARY_CLOSE, OnBtnSummaryClose)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BTN_SUMMARY_CONFIGURE, &CPatientSummaryDlg::OnBnClickedBtnSummaryConfigure)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPatientSummaryDlg message handlers

BOOL CPatientSummaryDlg::OnInitDialog() 
{
	try {

		CNxDialog::OnInitDialog();

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnConfigure.AutoSet(NXB_MODIFY);

		m_SummaryList = BindNxDataList2Ctrl(IDC_PATIENT_SUMMARY_LIST, GetRemoteData(), false);

		// (j.jones 2009-10-26 09:23) - PLID 32904 - added bulk caching
		// (j.gruber 2010-06-15 11:40) - PLID 26363 - added more
		// (j.jones 2012-02-01 17:32) - PLID 46145 - added GlobalPeriodSort
		g_propManager.CachePropertiesInBulk("CPackages", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'PackageTaxEstimation' OR "
			"Name = 'IncludeAllPrePaysInPopUps' OR "
			"Name LIKE 'PatSum%%Check' OR "
			"Name LIKE 'PatSum%%Sort' OR "
			"Name LIKE 'PatSum%%List' OR "
			"Name = 'GlobalPeriodSort' "
			"OR Name = 'GlobalPeriod_OnlySurgicalCodes' "	// (j.jones 2012-07-24 10:45) - PLID 51737
			"OR Name = 'GlobalPeriod_IgnoreModifier78' "	// (j.jones 2012-07-26 14:26) - PLID 51827
			")",
			_Q(GetCurrentUserName()));

		if(m_nPatientID == -1) {
			m_nPatientID = GetActivePatientID();
		}

		if(m_strPatientName.IsEmpty()) {
			m_strPatientName = GetExistingPatientName(m_nPatientID);
		}

		CString strWindowText;
		strWindowText.Format("Patient Summary for %s", m_strPatientName);
		SetWindowText(strWindowText);

		// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
		m_pList = BindNxDataList2Ctrl(IDC_PAT_INFO_LIST, false);

		if(GetMainFrame()) {
			// (b.spivey, May 21, 2012) - PLID 50558 - We use the default patient blue always.
			m_bkg.SetColor(GetNxColor(GNC_PATIENT_STATUS, 1));
		}

		// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
		LoadConfigValues();

		// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
		LoadCustomFieldData();

		//now load the list, but do it with a timer, so we can watch the information load on the screen
		SetTimer(TIMER_LOAD_SUMMARY_LIST, 100, NULL);
	
	}NxCatchAll("Error in CPatientSummaryDlg::OnInitDialog");
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
CPatSummaryConfigType CPatientSummaryDlg::GetConfigInfo(CString strBase, BOOL bDefaultCheck /*= FALSE*/, long nDefaultSort /*= -1*/, long nDefaultList /*= 1*/)
{
	CPatSummaryConfigType cfgType;

	cfgType.m_strConfigName = strBase;
	cfgType.m_bCheck = GetRemotePropertyInt(strBase + "Check", bDefaultCheck, 0, GetCurrentUserName(), true);
	cfgType.m_nList = GetRemotePropertyInt(strBase + "List", nDefaultList, 0, GetCurrentUserName(), true);
	cfgType.m_nSortOrder = GetRemotePropertyInt(strBase + "Sort", nDefaultSort, 0, GetCurrentUserName(), true);
	return cfgType;
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::LoadCustomFieldData()
{
	_RecordsetPtr rs = CreateRecordset("SELECT ID, Name FROM CustomFieldsT");

	while (!rs->eof) {

		long nID = AdoFldLong(rs, "ID");
		CString strName = AdoFldString(rs, "Name", "");

		m_mapCustomFields.SetAt(nID, strName);

		rs->MoveNext();
	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::LoadConfigValues()
{
	//It is ok to change the default sort valuess for these entries.
	//When these values are loaded, they will be displayed in order,
	//and then re-numbered to fix duplicate sorts.

	//default to column 1
	m_pConfigList.Add(GetConfigInfo("PatSumPatInfo", TRUE, 1, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumEmergencyContactInfo", TRUE, 2, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumInsPartyInfo", TRUE, 3, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumPatNotes", TRUE, 4, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumMainPhysician", TRUE, 5, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumFirstContactDate", TRUE, 6, 1));
	
	m_pConfigList.Add(GetConfigInfo("PatSumCustom1", FALSE, 7, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumCustom2", FALSE, 8, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumCustom3", FALSE, 9, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumCustom4", FALSE, 10, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumRace", FALSE, 11, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumEthnicity", FALSE, 12, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumLanguage", FALSE, 13, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumDiagCodes", FALSE, 14, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumCurIllDate", FALSE, 15, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumPatType", FALSE, 16, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumLocation", FALSE, 17, 1));	
	m_pConfigList.Add(GetConfigInfo("PatSumWarning", FALSE, 18, 1));
	m_pConfigList.Add(GetConfigInfo("PatSumLastModified", FALSE, 19, 1));

	//default to column 2
	m_pConfigList.Add(GetConfigInfo("PatSumEmploymentInfo", TRUE, 1, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumRefInfo", TRUE, 2, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumPatBal", TRUE, 3, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumInsBal", TRUE, 4, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumLastDiagsBilled", TRUE, 5, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumLastApptInfo", TRUE, 6, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCurMeds", TRUE, 7, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumAllergies", TRUE, 8, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumPatCoord", TRUE, 9, 2));	
	m_pConfigList.Add(GetConfigInfo("PatSumEnteredBy", TRUE, 10, 2));
	// (j.jones 2012-02-01 17:16) - PLID 46145 - added global periods
	m_pConfigList.Add(GetConfigInfo("PatSumGlobalPeriods", FALSE, 11, 2));
	
	m_pConfigList.Add(GetConfigInfo("PatSumCustomText1", FALSE, 12, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomText2", FALSE, 13, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomText3", FALSE, 14, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomText4", FALSE, 15, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomText5", FALSE, 16, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomText6", FALSE, 17, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomText7", FALSE, 18, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomText8", FALSE, 19, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomText9", FALSE, 20, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomText10", FALSE, 21, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomText11", FALSE, 22, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomText12", FALSE, 23, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomNote", FALSE, 24, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomList1", FALSE, 25, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomList2", FALSE, 26, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomList3", FALSE, 27, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomList4", FALSE, 28, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomList5", FALSE, 29, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomList6", FALSE, 30, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomContact", FALSE, 31, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomCheck1", FALSE, 32, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomCheck2", FALSE, 33, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomCheck3", FALSE, 34, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomCheck4", FALSE, 35, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomCheck5", FALSE, 36, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomCheck6", FALSE, 37, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomDate1", FALSE, 38, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomDate2", FALSE, 39, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomDate3", FALSE, 40, 2));
	m_pConfigList.Add(GetConfigInfo("PatSumCustomDate4", FALSE, 41, 2));	
	
	
}

void CPatientSummaryDlg::OnTimer(UINT nIDEvent) 
{
	try {

		switch(nIDEvent) {
		case TIMER_LOAD_SUMMARY_LIST:
			{
				KillTimer(TIMER_LOAD_SUMMARY_LIST);

				DontShowMeAgain(this, GetHelpText(), "PatientSummaryDlg", "Patient Summary", FALSE, FALSE, FALSE);

				// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
				LoadTopList();
				LoadBottomList();
			}
			break;
		}

	}NxCatchAll("Error in CPatientSummaryDlg::OnTimer");

	CNxDialog::OnTimer(nIDEvent);
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
long CPatientSummaryDlg::GetCheckedValue(CString strBase) 
{

	CPatSummaryConfigType cfgType = GetConfigInfo(strBase);

	if (cfgType.m_bCheck) {
		return 1;
	}
	else {
		return 0;
	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
CString CPatientSummaryDlg::GetTotalSql() 
{
	CString strSql = BeginSqlBatch();
	AddDeclarationToSqlBatch(strSql, "DECLARE @nPatientID INT; \r\n");
	AddStatementToSqlBatch(strSql, "SET @nPatientID = %li \r\n ", m_nPatientID);

	long nCheckValue = GetCheckedValue("PatSumPatInfo");
	
	AddStatementToSqlBatch(strSql, "SELECT UserDefinedID, First, Middle, Last, Address1, Address2, City, State, Zip, Country,  \r\n "
		"  HomePhone, WorkPhone, Extension, CellPhone, Fax, Email, Pager, OtherPhone, PreferredContact, \r\n "
		"  PrivHome, PrivWork, PrivCell, PrivFax, PrivEmail, PrivPager, PrivOther,   \r\n "
		"  SocialSecurity, MaritalStatus, SpouseName, Birthdate, PersonT.Gender, DeclinedEmail, PrefixT.Prefix, PersonT.Title, PatientsT.NickName \r\n "
		"  FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n "
		"  LEFT JOIN PrefixT ON PersonT.PrefixID = PrefixT.ID \r\n "
		"  WHERE PersonT.ID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumEmergencyContactInfo");
	AddStatementToSqlBatch(strSql, "SELECT EmergFirst, EmergLast, EmergHPhone, EmergWphone, EmergRelation  \r\n "
		"  FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n "
		"  WHERE PersonT.ID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustom1");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 1 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustom2");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 2 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustom3");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 3 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustom4");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 4 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumPatNotes");
	AddStatementToSqlBatch(strSql, "SELECT Note as Field \r\n "
		"  FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n "
		"  WHERE PersonT.ID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumMainPhysician");
	AddStatementToSqlBatch(strSql, "SELECT (PersonProvT.Last + ', ' + PersonProvT.First + ' ' + PersonProvT.Middle) as Field  \r\n "
		"  FROM PatientsT LEFT JOIN PersonT PersonProvT ON PatientsT.MainPhysician = PersonProvT.ID \r\n "
		"  WHERE PatientsT.PersonID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumPatCoord");
	AddStatementToSqlBatch(strSql, "SELECT (PersonT.Last + ', ' + PersonT.First) as Field   \r\n "
		"  FROM PatientsT LEFT JOIN PersonT ON PatientsT.EmployeeID = PersonT.ID \r\n "
		"  WHERE PatientsT.PersonID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumFirstContactDate");
	AddStatementToSqlBatch(strSql, "SELECT FirstContactDate as Date \r\n "
		"  FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n "
		"  WHERE PersonT.ID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumEnteredBy");
	AddStatementToSqlBatch(strSql, "SELECT UsersT.UserName as Field \r\n "
		"  FROM PersonT LEFT JOIN UsersT ON PersonT.UserID = UsersT.PersonID \r\n "
		"  WHERE PersonT.ID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);

	// (j.jones 2012-02-01 17:16) - PLID 46145 - added global periods
	nCheckValue = GetCheckedValue("PatSumGlobalPeriods");
	int GlobalPeriodSort = GetRemotePropertyInt("GlobalPeriodSort",0,0,"<None>", TRUE);

	// (j.jones 2012-07-24 10:45) - PLID 51737 - added a preference to only track global periods for
	// surgical codes only, if it is disabled when we would look at all codes
	long nSurgicalCodesOnly = GetRemotePropertyInt("GlobalPeriod_OnlySurgicalCodes", 1, 0, "<None>", true);

	// (j.jones 2012-07-26 15:22) - PLID 51827 - added another preference to NOT track global periods
	// if the charge uses modifier 78
	long nIgnoreModifier78 = GetRemotePropertyInt("GlobalPeriod_IgnoreModifier78", 1, 0, "<None>", true);

	// (j.jones 2012-07-24 10:45) - PLID 51737 - optionally filter on surgical codes only
	// (j.jones 2012-07-26 15:22) - PLID 51827 - optionally exclude modifier 78
	AddStatementToSqlBatch(strSql, "SELECT Code + ' through ' + Convert(nvarchar, DATEADD(day,GlobalPeriod,LineItemT.Date), 1) AS Field "
			"FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"INNER JOIN CPTCodeT ON ChargesT.ServiceID = CPTCodeT.ID "
			"INNER JOIN ServiceT ON CPTCodeT.ID = ServiceT.ID "
			"LEFT JOIN ServicePayGroupsT ON ServiceT.PayGroupID = ServicePayGroupsT.ID "
			"LEFT JOIN (SELECT OriginalLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_OriginalLineItemsQ ON LineItemT.ID = LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID "
			"LEFT JOIN (SELECT VoidingLineItemID FROM LineItemCorrectionsT) AS LineItemCorrections_VoidingLineItemsQ ON LineItemT.ID = LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 10 AND GlobalPeriod Is Not Null AND CPTCodeT.GlobalPeriod <> 0 "
			"AND LineItemCorrections_OriginalLineItemsQ.OriginalLineItemID Is Null "
			"AND LineItemCorrections_VoidingLineItemsQ.VoidingLineItemID Is Null "
			"AND DATEADD(day,GlobalPeriod,dbo.AsDateNoTime(LineItemT.Date)) > GetDate() "
			"AND LineItemT.PatientID = @nPatientID AND (%li = 1) "
			"AND (%li <> 1 OR ServicePayGroupsT.Category = %li) "
			"AND (%li <> 1 OR (Coalesce(ChargesT.CPTModifier, '') <> '78' AND Coalesce(ChargesT.CPTModifier2, '') <> '78' AND Coalesce(ChargesT.CPTModifier3, '') <> '78' AND Coalesce(ChargesT.CPTModifier4, '') <> '78')) "
			"ORDER BY DATEADD(day,GlobalPeriod,LineItemT.Date) %s",
			nCheckValue,
			nSurgicalCodesOnly, PayGroupCategory::SurgicalCode, nIgnoreModifier78,
			GlobalPeriodSort == 0 ? "ASC" : "DESC");

	nCheckValue = GetCheckedValue("PatSumEmploymentInfo");	
	AddStatementToSqlBatch(strSql, "SELECT Occupation, Company, Employment, EmployerFirst, EmployerMiddle, \r\n "
		"  EmployerLast, EmployerAddress1, EmployerAddress2, EmployerCity, EmployerState, EmployerZip \r\n "
		"  FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n "
		"  WHERE PersonT.ID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumRace");	
	// (b.spivey, May 28, 2013) - PLID 55937 - Need to display the list of races using the new structure instead of a singular race with the old structure. 
	AddStatementToSqlBatch(strSql, "SELECT LEFT(RaceSubQ.RaceName, LEN(RaceSubQ.RaceName) -1) AS Field "
		"	FROM PersonT "
		"	CROSS APPLY "
		"	( "
		"		SELECT ( "
		"			SELECT RT.Name + ', ' "
		"			FROM PersonRaceT PRT "
		"			INNER JOIN RaceT RT ON PRT.RaceID = RT.ID "
		"			WHERE PRT.PersonID = PersonT.ID "
		"			FOR XML PATH(''), TYPE "
		"		).value('/','nvarchar(max)') "
		"	) RaceSubQ (RaceName) "
		"	WHERE PersonT.ID = @nPatientID AND (%li = 1) ", nCheckValue);

	// (a.walling 2011-12-08 13:25) - PLID 46951 - This was incorrectly using PatSumRace instead of PatSumEthnicity
	// (d.thompson 2012-08-09) - PLID 52062 - Reworked ethnicity table structure
	nCheckValue = GetCheckedValue("PatSumEthnicity");	
	AddStatementToSqlBatch(strSql, "SELECT  EthnicityT.Name  as Field \r\n "
		"   FROM PersonT \r\n"
		"	LEFT JOIN EthnicityT ON PersonT.Ethnicity = EthnicityT.ID \r\n"
		"	LEFT JOIN EthnicityCodesT ON EthnicityT.EthnicityCodeID = EthnicityCodesT.ID \r\n"
		"  WHERE PersonT.ID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);	

	nCheckValue = GetCheckedValue("PatSumLanguage");	
	// (d.thompson 2012-08-14) - PLID 52046 - Reworked language table structure
	AddStatementToSqlBatch(strSql, "SELECT  LanguageT.Name as Field \r\n "
		"   FROM PersonT  \r\n "
		"	LEFT JOIN LanguageT ON PersonT.LanguageID = LanguageT.ID \r\n"
		"  WHERE PersonT.ID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);	
	
	// (s.tullis 2014-03-06 10:07) - PLID 60762 - 40090 - Patients Module > NexEMR > Pt. Summary > Default Diagnosis Codes (in top ½)	
	nCheckValue = GetCheckedValue("PatSumDiagCodes");	
	AddStatementToSqlBatch(strSql,"SELECT PatientsT.PersonID, '1' AS IndexNumber, \r\n "
			"DiagCodes9.ID AS DiagICD9CodeID, DiagCodes9.CodeNumber +' - '+ DiagCodes9.CodeDesc AS DefaultICD9, \r\n "
			"DiagCodes10.ID AS DiagICD10CodeID, DiagCodes10.CodeNumber+' - '+DiagCodes10.CodeDesc AS DefaultICD10 \r\n "
			"FROM PatientsT \r\n "
			"LEFT JOIN DiagCodes DiagCodes9 ON PatientsT.DefaultDiagID1 = DiagCodes9.ID \r\n "
			"LEFT JOIN DiagCodes DiagCodes10 ON PatientsT.DefaultICD10DiagID1 = DiagCodes10.ID \r\n "
			"WHERE (DiagCodes9.ID Is Not Null OR DiagCodes10.ID Is Not Null) AND PatientsT.PersonID = @nPatientID AND (%li = 1) \r\n "
			"UNION ALL \r\n "
			"SELECT PatientsT.PersonID, '2' AS IndexNumber, \r\n "
			"DiagCodes9.ID AS DiagICD9CodeID, DiagCodes9.CodeNumber +' - '+ DiagCodes9.CodeDesc AS DefaultICD9, \r\n "
			"DiagCodes10.ID AS DiagICD10CodeID, DiagCodes10.CodeNumber+' - '+DiagCodes10.CodeDesc AS DefaultICD10 \r\n "
			"FROM PatientsT "
			"LEFT JOIN DiagCodes DiagCodes9 ON PatientsT.DefaultDiagID2 = DiagCodes9.ID \r\n "
			"LEFT JOIN DiagCodes DiagCodes10 ON PatientsT.DefaultICD10DiagID2 = DiagCodes10.ID \r\n "
			"WHERE (DiagCodes9.ID Is Not Null OR DiagCodes10.ID Is Not Null) AND PatientsT.PersonID = @nPatientID AND (%li = 1) \r\n "
			"UNION ALL \r\n "
			"SELECT PatientsT.PersonID, '3' AS IndexNumber, \r\n "
			"DiagCodes9.ID AS DiagICD9CodeID, DiagCodes9.CodeNumber +' - '+ DiagCodes9.CodeDesc AS DefaultICD9, \r\n "
			"DiagCodes10.ID AS DiagICD10CodeID, DiagCodes10.CodeNumber+' - '+DiagCodes10.CodeDesc AS DefaultICD10 \r\n "
			"FROM PatientsT \r\n "
			"LEFT JOIN DiagCodes DiagCodes9 ON PatientsT.DefaultDiagID3 = DiagCodes9.ID \r\n "
			"LEFT JOIN DiagCodes DiagCodes10 ON PatientsT.DefaultICD10DiagID3 = DiagCodes10.ID \r\n "
			"WHERE (DiagCodes9.ID Is Not Null OR DiagCodes10.ID Is Not Null) AND PatientsT.PersonID = @nPatientID AND (%li = 1) "
			"UNION ALL \r\n "
			"SELECT PatientsT.PersonID, '4' AS IndexNumber, \r\n "
			"DiagCodes9.ID AS DiagICD9CodeID, DiagCodes9.CodeNumber +' - '+ DiagCodes9.CodeDesc AS DefaultICD9, \r\n "
			"DiagCodes10.ID AS DiagICD10CodeID, DiagCodes10.CodeNumber+' - '+DiagCodes10.CodeDesc AS DefaultICD10\r\n "
			"FROM PatientsT \r\n "
			"LEFT JOIN DiagCodes DiagCodes9 ON PatientsT.DefaultDiagID4 = DiagCodes9.ID \r\n "
			"LEFT JOIN DiagCodes DiagCodes10 ON PatientsT.DefaultICD10DiagID4 = DiagCodes10.ID \r\n "
			"WHERE (DiagCodes9.ID Is Not Null OR DiagCodes10.ID Is Not Null) AND PatientsT.PersonID = @nPatientID AND (%li = 1); \r\n ", nCheckValue, nCheckValue, nCheckValue, nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCurIllDate");
	AddStatementToSqlBatch(strSql, "SELECT DefaultInjuryDate as Date \r\n "
		"  FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n "
		"  WHERE PersonT.ID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumPatType");
	AddStatementToSqlBatch(strSql, "SELECT GroupName as Field \r\n "
		"  FROM PatientsT LEFT JOIN GroupTypes ON PatientsT.TypeOfPatient = GroupTypes.TypeIndex \r\n "
		"  WHERE PatientsT.PersonID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumLocation");
	AddStatementToSqlBatch(strSql, "SELECT Name as Field \r\n "
		"  FROM PersonT LEFT JOIN LocationsT ON PersonT.Location = LocationsT.ID \r\n "
		"  WHERE PersonT.ID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumRefInfo");	
	AddStatementToSqlBatch(strSql, /*"SELECT Name, 1 as Type \r\n "
		" FROM PatientsT LEFT JOIN ReferralSourceT ON PatientsT.ReferralID = ReferralSourceT.PersonID \r\n "
		" WHERE PatientsT.PersonID = @nPatientID AND (%li = 1) \r\n "
		" UNION ALL \r\n "*/
		" SELECT Name, 2 as Type FROM  \r\n "
		" PatientsT LEFT JOIN MultiReferralsT ON PatientsT.PersonID = MultiReferralsT.PatientID \r\n "
		" LEFT JOIN ReferralSourceT ON MultiReferralsT.ReferralID = ReferralSourceT.PersonID \r\n "
		" WHERE PatientsT.PersonID = @nPatientID AND (%li = 1)   \r\n "
		" UNION ALL \r\n "
		" SELECT (RefProvT.Last + ', ' + RefProvT.First + ' ' + RefProvT.Middle), 3 as Type FROM  \r\n "
		" PatientsT LEFT JOIN PersonT RefProvT ON PatientsT.DefaultReferringPhyID = RefProvT.ID \r\n "
		" WHERE PatientsT.PersonID = @nPatientID AND (%li = 1)   \r\n "
		" UNION ALL \r\n "
		" SELECT (RefPatT.Last + ', ' + RefPatT.First + ' ' + RefPatT.Middle), 4 as Type FROM  \r\n "
		" PatientsT LEFT JOIN PersonT RefPatT ON PatientsT.ReferringPatientID = RefPatT.ID \r\n "
		" WHERE PatientsT.PersonID = @nPatientID AND (%li = 1)   \r\n "
		" UNION ALL \r\n "
		" SELECT (PCPProvT.Last + ', ' + PCPProvT.First + ' ' + PCPProvT.Middle), 5 as Type FROM  \r\n "
		" PatientsT LEFT JOIN PersonT PCPProvT ON PatientsT.PCP = PCPProvT.ID \r\n "
		" WHERE PatientsT.PersonID = @nPatientID AND (%li = 1);   \r\n ", /*nCheckValue,*/ nCheckValue, nCheckValue, nCheckValue, nCheckValue);

	nCheckValue = GetCheckedValue("PatSumWarning");
	AddStatementToSqlBatch(strSql, "SELECT WarningMessage, DisplayWarning, WarningUseExpireDate, \r\n "
		" WarningExpireDate, UsersT.UserName, WarningUserID  \r\n "
		"  FROM PatientsT LEFT JOIN PersonT ON PatientsT.PersonID = PersonT.ID \r\n "
		"  LEFT JOIN UsersT ON PersonT.WarningUserID = UsersT.PersonID \r\n "
		"  WHERE PatientsT.PersonID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);
			
	nCheckValue = GetCheckedValue("PatSumLastModified");
	AddStatementToSqlBatch(strSql, "SELECT PatientsT.ModifiedDate as Date \r\n "
		"  FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID \r\n "
		"  WHERE PersonT.ID = @nPatientID AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomText1");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 11 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomText2");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 12 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomText3");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 13 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomText4");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 14 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomText5");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 15 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomText6");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 16 AND (%li = 1); \r\n ", nCheckValue);
	
	nCheckValue = GetCheckedValue("PatSumCustomText7");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 90 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomText8");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 91 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomText9");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 92 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomText10");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 93 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomText11");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 94 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomText12");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 95 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomNote");
	AddStatementToSqlBatch(strSql, "SELECT TextParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 17 AND (%li = 1); \r\n ", nCheckValue);

	// (j.armen 2011-06-27 16:57) - PLID 44253 - updated to use new custom list data structure
	nCheckValue = GetCheckedValue("PatSumCustomList1");
	AddStatementToSqlBatch(strSql, "SELECT dbo.GetCustomList(@nPatientID, 21) as Field \r\n "
		"  WHERE (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomList2");
	AddStatementToSqlBatch(strSql, "SELECT dbo.GetCustomList(@nPatientID, 22) as Field \r\n "
		"  WHERE (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomList3");
	AddStatementToSqlBatch(strSql, "SELECT dbo.GetCustomList(@nPatientID, 23) as Field \r\n "
		"  WHERE (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomList4");
	AddStatementToSqlBatch(strSql, "SELECT dbo.GetCustomList(@nPatientID, 24) as Field \r\n "
		"  WHERE (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomList5");
	AddStatementToSqlBatch(strSql, "SELECT dbo.GetCustomList(@nPatientID, 25) as Field \r\n "
		"  WHERE (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomList6");
	AddStatementToSqlBatch(strSql, "SELECT dbo.GetCustomList(@nPatientID, 26) as Field \r\n "
		"  WHERE (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomContact");
	AddStatementToSqlBatch(strSql, "SELECT Last + ', ' + First + ' ' + Middle as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  LEFT JOIN PersonT ON CustomFieldDataT.IntParam = PersonT.ID \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 31 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomCheck1");
	AddStatementToSqlBatch(strSql, "SELECT CASE WHEN IntParam = 0 then 'False' ELSE 'True' END as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 41 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomCheck2");
	AddStatementToSqlBatch(strSql, "SELECT CASE WHEN IntParam = 0 then 'False' ELSE 'True' END as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 42 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomCheck3");
	AddStatementToSqlBatch(strSql, "SELECT CASE WHEN IntParam = 0 then 'False' ELSE 'True' END as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 43 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomCheck4");
	AddStatementToSqlBatch(strSql, "SELECT CASE WHEN IntParam = 0 then 'False' ELSE 'True' END as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 44 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomCheck5");
	AddStatementToSqlBatch(strSql, "SELECT CASE WHEN IntParam = 0 then 'False' ELSE 'True' END as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 45 AND (%li = 1); \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCustomCheck6");
	AddStatementToSqlBatch(strSql, "SELECT CASE WHEN IntParam = 0 then 'False' ELSE 'True' END as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 46 AND (%li = 1); \r\n ", nCheckValue);
	
	nCheckValue = GetCheckedValue("PatSumCustomDate1");
	AddStatementToSqlBatch(strSql, "SELECT DateParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 51 AND (%li = 1); \r\n ", nCheckValue);	

	nCheckValue = GetCheckedValue("PatSumCustomDate2");
	AddStatementToSqlBatch(strSql, "SELECT DateParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 52 AND (%li = 1); \r\n ", nCheckValue);	

	nCheckValue = GetCheckedValue("PatSumCustomDate3");
	AddStatementToSqlBatch(strSql, "SELECT DateParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 53 AND (%li = 1); \r\n ", nCheckValue);	

	nCheckValue = GetCheckedValue("PatSumCustomDate4");
	AddStatementToSqlBatch(strSql, "SELECT DateParam as Field \r\n "
		"  FROM CustomFieldDataT \r\n "
		"  WHERE CustomFieldDataT.PersonID = @nPatientID AND FieldID = 54 AND (%li = 1); \r\n ", nCheckValue);	
	

	// (j.gruber 2010-08-03 12:58) - PLID 39948 - changed copay structure
	nCheckValue = GetCheckedValue("PatSumInsPartyInfo");
	AddStatementToSqlBatch(strSql, "SELECT First, Middle, Last, Address1, Address2, City, State, Zip, Employer as Company, \r\n "
		" BirthDate, SocialSecurity, HomePhone, RelationtoPatient, Gender, \r\n "
		" DateOfCurAcc, AccidentType, AccidentState, \r\n "
		" InsuranceCoT.Name as CompanyName, InsurancePlansT.PlanName as InsPlan, InsurancePlansT.PlanType, \r\n "
		" RespTypeT.TypeName, \r\n "
		" IDForInsurance, PolicyGroupNum, Copay, CopayPercent \r\n "
		" FROM (SELECT InsuredPartyT.*, InsPartyPayGroupsT.CopayPercentage as CopayPercent, InsPartyPayGroupsT.CopayMoney as Copay FROM InsuredPartyT LEFT JOIN (SELECT InsuredPartyID, CopayMoney, CopayPercentage FROM "
		" (SELECT * FROM ServicePayGroupsT WHERE Name = 'Copay') ServicePayGroupsT "
		" LEFT JOIN InsuredPartyPayGroupsT ON ServicePayGroupsT.ID = InsuredPartyPayGroupsT.PayGroupID) InsPartyPayGroupsT "
		" ON InsuredPartyT.PersonID = InsPartyPayGroupsT.InsuredPartyID ) InsuredPartyT LEFT JOIN PersonT ON InsuredPartyT.PersonID = PersonT.ID \r\n "
		" LEFT JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID \r\n "
		" LEFT JOIN InsurancePlansT ON InsuredPartyT.InsPlan = InsurancePlansT.ID \r\n "
		" LEFT JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID \r\n "
		" WHERE InsuredPartyT.PatientID = @nPatientID AND (%li = 1) Order By CASE WHEN RespTypeT.Priority < 0 THEN 9999 ELSE RespTypeT.Priority END ASC \r\n ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumCurMeds");
	AddStatementToSqlBatch(strSql, "SELECT Discontinued, Data FROM  \r\n "
		" CurrentPatientMedsT LEFT JOIN DrugList ON CurrentPatientMedsT.MedicationID = DrugList.ID  \r\n "
		" LEFT JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID \r\n "
		" WHERE CurrentPatientMedsT.PatientID = @nPatientID AND (%li = 1) \r\n ", nCheckValue);		
	
	nCheckValue = GetCheckedValue("PatSumAllergies");
	AddStatementToSqlBatch(strSql, "SELECT data, Discontinued \r\n "
		" FROM PatientAllergyT Left Join AllergyT on AllergyT.ID = PatientAllergyT.AllergyID  \r\n "
		" Left Join EmrDataT ON EmrDataT.ID = AllergyT.EmrDataID \r\n "
		" WHERE PatientAllergyT.PersonID = @nPatientID AND (%li = 1) \r\n ", nCheckValue);
		
	nCheckValue = GetCheckedValue("PatSumLastApptInfo");
	// (j.jones 2010-08-11 17:02) - PLID 38508 - this logic intentionally loads the last appt. prior to today's date
	AddStatementToSqlBatch(strSql, "SELECT TOP 1 "
					"ResBasicQ.StartTime, "
					"dbo.GetResourceString(ResBasicQ.ID) AS Resource, "
					"CASE WHEN ResBasicQ.AptPurposeName <> '' THEN AptTypeT.Name + ' - ' + ResBasicQ.AptPurposeName ELSE AptTypeT.Name END AS Purpose "
					"FROM (SELECT AppointmentsT.ID, AppointmentsT.PatientID, AppointmentsT.AptTypeID, "
					"	dbo.GetPurposeString(AppointmentsT.ID) AS AptPurposeName, "
					"	AppointmentsT.Date, AppointmentsT.StartTime, AppointmentsT.Status, AppointmentsT.ShowState "
					"	FROM AppointmentsT "
					"	LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
					"	WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3) ResBasicQ "
					"INNER JOIN AptTypeT ON ResBasicQ.AptTypeID = AptTypeT.ID "
					"WHERE PatientID = @nPatientID "
					"AND dbo.AsDateNoTime(ResBasicQ.Date) < dbo.AsDateNoTime(GetDate()) AND (%li = 1) "
					"ORDER BY StartTime DESC", nCheckValue);
					// (s.tullis 2014-03-06 17:59) - PLID 60761 - 40080 - Patients Module > NexEMR > Pt. Summary > Last Diagnosis Codes Billed (in top ½)
	nCheckValue = GetCheckedValue("PatSumLastDiagsBilled");
	AddStatementToSqlBatch(strSql, "DECLARE @MostRecentBillID INT "
					"SET NOCOUNT ON "
					"SET @MostRecentBillID = ( "
					"SELECT TOP 1 BillsT.ID "
					"FROM BillsT  "
					"WHERE BillsT.Deleted = 0 AND EntryType = 1  "
					"AND BillsT.PatientID =@nPatientID "
					"ORDER BY BillsT.Date DESC, BillsT.ID DESC ) "
					"SET NOCOUNT OFF "
					"SELECT BillsT.ID, BillDiagCodet.OrderIndex, ICD9.CodeNumber AS ICD9Code,  ICD10.CodeNumber AS ICD10Code "
					"From BillsT "
					"left join BillDiagCodeT "
					"On BillsT.ID = BillDiagCodeT.BillID " 
					"left join Diagcodes ICD9 "
					"On BillDiagCodeT.ICD9DiagID= ICD9.ID "
					"left join Diagcodes ICD10 "
					"On BillDiagCodeT.ICD10DiagID= ICD10.ID "
					"WHERE BillsT.ID = @MostRecentBillID AND (%li=1)"
					" ORDER BY BillDiagCodeT.OrderIndex ", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumPatBal");
	AddStatementToSqlBatch(strSql, "SELECT "
					"Convert(money, Coalesce(TotalPatientCharges, 0) - Coalesce(TotalPatientPayments, 0)) AS Currency "					
					"FROM PersonT "
					"LEFT JOIN ("
					"	SELECT BillsT.PatientID, Sum(Round(Convert(money,ChargeRespT.Amount),2)) AS TotalPatientCharges "
					"	FROM BillsT "
					"	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
					"	INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
					"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"	WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
					"	AND LineItemT.Type = 10 AND BillsT.EntryType = 1 "
					"	AND (ChargeRespT.InsuredPartyID Is Null OR ChargeRespT.InsuredPartyID = -1) "
					"	GROUP BY BillsT.PatientID "
					"	) AS PatientChargeTotalQ ON PersonT.ID = PatientChargeTotalQ.PatientID "					
					"LEFT JOIN ("
					"	SELECT LineItemT.PatientID, Sum(LineItemT.Amount) AS TotalPatientPayments "
					"	FROM PaymentsT "
					"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					"	WHERE LineItemT.Deleted = 0 "
					"	AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 "
					"	AND (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID = -1) "
					"	GROUP BY LineItemT.PatientID "
					"	) AS PatientPaymentTotalQ ON PersonT.ID = PatientPaymentTotalQ.PatientID "					
					"WHERE PersonT.ID = @nPatientID AND (%li = 1)", nCheckValue);

	nCheckValue = GetCheckedValue("PatSumInsBal");
	AddStatementToSqlBatch(strSql, "SELECT "					
					"Convert(money, Coalesce(TotalInsuranceCharges, 0) - Coalesce(TotalInsurancePayments, 0)) AS Currency "
					"FROM PersonT "					
					"LEFT JOIN ("
					"	SELECT BillsT.PatientID, Sum(Round(Convert(money,ChargeRespT.Amount),2)) AS TotalInsuranceCharges "
					"	FROM BillsT "
					"	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
					"	INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
					"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"	WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
					"	AND LineItemT.Type = 10 AND BillsT.EntryType = 1 "
					"	AND ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.InsuredPartyID <> -1 "
					"	GROUP BY BillsT.PatientID "
					"	) AS InsuranceChargeTotalQ ON PersonT.ID = InsuranceChargeTotalQ.PatientID "
					"LEFT JOIN ("
					"	SELECT LineItemT.PatientID, Sum(LineItemT.Amount) AS TotalInsurancePayments "
					"	FROM PaymentsT "
					"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					"	WHERE LineItemT.Deleted = 0 "
					"	AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 "
					"	AND PaymentsT.InsuredPartyID Is Not Null AND PaymentsT.InsuredPartyID <> -1 "
					"	GROUP BY LineItemT.PatientID "
					"	) AS InsurancePaymentTotalQ ON PersonT.ID = InsurancePaymentTotalQ.PatientID "
					"WHERE PersonT.ID = @nPatientID AND (%li = 1)", nCheckValue);	

	return strSql;

}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::AddListDataToArray(CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *aryObjects, 
										CString strLabel /*= ""*/, COLORREF cColor /*= RGB(0,0,0)*/, CString strTabLink /*=""*/)
{
	//let exceptions be thrown to the caller

	if(aryObjects == NULL) {
		return;
	}

	CPatSummaryListInfoObject *pNew = new CPatSummaryListInfoObject(strLabel, cColor, strTabLink);
	aryObjects->Add(pNew);
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayCustomField(ADODB::_RecordsetPtr rsAllRecs, long nCustomField, CString strBase)
{
	CString strFieldName;
	if (m_mapCustomFields.Lookup(nCustomField, strFieldName)) {

		CPatSummaryConfigType cfgType = GetConfigInfo(strBase);
		if (cfgType.m_bCheck) {

			CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryCustomField = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;

			CString strTabLink;
			if (nCustomField < 5) {
				strTabLink = "tabG1";
			}
			else {
				strTabLink = "tabCustom";
			}	

			//add the top link
			AddListDataToArray(paryCustomField, strFieldName, RGB(0,0,192), strTabLink);

			if (rsAllRecs->eof) {			
				AddListDataToArray(paryCustomField, "<None>");
			}

			if (!rsAllRecs->eof) {			

				CString strValue;
				COleDateTime dtInvalid, dt;
				dtInvalid.SetStatus(COleDateTime::invalid);

				//see if its a date custom field
				switch (nCustomField) {
					case 51:
					case 52:
					case 53:
					case 54:
						{
						dt = AdoFldDateTime(rsAllRecs, "Field", dtInvalid);
						CString strDt = FormatDateTimeForInterface(dt, NULL, dtoDate); 
						if ((dt.GetStatus() != COleDateTime::invalid) && strDt.CompareNoCase("Invalid DateTime") != 0) {
							strValue = strDt;
						}
						else {
							strValue = "";
						}
						}
					break;

					default:
						strValue = AdoFldString(rsAllRecs, "Field", "");
					break;
				}

				if (strValue.IsEmpty()) {
					strValue = "<None>";
				}

				//now add our field
				AddListDataToArray(paryCustomField, strValue);
				
			}		
			

			//now a blank row
			AddListDataToArray(paryCustomField);


			//now add our array to our main array		
			if (cfgType.m_nList == 1) {
				//put it in the left list				
				CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryCustomField);
				m_aryLeftList.Add(paryList);
			}
			else {
				CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryCustomField);
				m_aryRightList.Add(paryList);
			}		
		}

	}
	else {	
		//how could we not find it?
		ASSERT(FALSE);	
	}	
}


// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayPatientInformation(ADODB::_RecordsetPtr rsAllRecs)
{
	if (!rsAllRecs->eof) {

		//Format is:
		//Prefix First Middle Last Title
		//Address1
		//Address2
		//City, State Zip Country
		//NickName
		//SSN
		//BirthDate (Age)
		//Marital Status - SpouseName
		//Gender
		//HomePhone
		//WorkPhone ext:Ext
		//CellPhone		
		//Pager
		//Other
		//Fax
		//Email
		//PrefContact		
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);

		FieldsPtr flds = rsAllRecs->Fields;

		CString strPreFix = AdoFldString(flds, "PreFix", "");
		CString strTitle = AdoFldString(flds, "Title", "");
		CString strNickName = AdoFldString(flds, "NickName", "");
		CString strFirst = AdoFldString(flds, "First", "");
		CString strMiddle = AdoFldString(flds, "Middle", "");
		CString strLast = AdoFldString(flds, "Last", "");
		CString strAdd1 = AdoFldString(flds, "Address1", "");
		CString strAdd2 = AdoFldString(flds, "Address2", "");
		CString strCity = AdoFldString(flds, "City", "");
		CString strState = AdoFldString(flds, "State", "");
		CString strZip = AdoFldString(flds, "Zip", "");
		CString strCountry = AdoFldString(flds, "Country", "");
		CString strSSN = AdoFldString(flds, "SocialSecurity", "");

		// (f.dinatale 2011-01-27) - PLID 33753 - Added permission checking to mask the SSN for the Patient Summary dialog.
		if(CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
			strSSN = FormatSSNText(strSSN, eSSNNoMask, "###-##-####");
		} else {
			if(CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && !CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
				strSSN = FormatSSNText(strSSN, eSSNPartialMask, "###-##-####");
			} else {
				strSSN = "";
			}
		}

		BOOL bDeclinedEmail = AdoFldBool(flds, "DeclinedEmail", FALSE);
		COleDateTime dtBirthDate = AdoFldDateTime(flds, "Birthdate", dtInvalid);
		CString strAge;
		if (dtBirthDate.GetStatus() != COleDateTime::invalid) {
			// (j.dinatale 2010-10-13) - PLID 38575 - need to call GetPatientAgeOnDate which no longer does any validation, 
			//  validation should only be done when bdays are entered/changed
			strAge = GetPatientAgeOnDate(dtBirthDate, COleDateTime::GetCurrentTime(), TRUE);
		}
		long nMaritalStatus = atoi(AdoFldString(flds, "MaritalStatus", "0"));
		CString strSpouseName = AdoFldString(flds, "Spousename", "");
		long nGender = AdoFldByte(flds, "Gender", 0);
		CString strHome = AdoFldString(flds, "HomePhone", "");
		CString strWorkPhone = AdoFldString(flds, "WorkPhone", "");
		CString strExt = AdoFldString(flds, "Extension", "");
		CString strCellPhone = AdoFldString(flds, "CellPhone", "");
		CString strPager = AdoFldString(flds, "Pager", "");
		CString strOther = AdoFldString(flds, "OtherPhone", "");
		CString strFax = AdoFldString(flds, "fax", "");
		CString strEmail = AdoFldString(flds, "Email", "");
		long nPrefContact  = AdoFldLong(flds, "PreferredContact", 0);
		BOOL bHomePriv = AdoFldBool(flds, "PrivHome", false);
		BOOL bWorkPriv = AdoFldBool(flds, "PrivWork", false);
		BOOL bCellPriv = AdoFldBool(flds, "PrivCell", false);
		BOOL bPagerPriv = AdoFldBool(flds, "PrivPager", false);
		BOOL bOtherPriv = AdoFldBool(flds, "PrivOther", false);
		BOOL bFaxPriv = AdoFldBool(flds, "PrivFax", false);
		BOOL bEmailPriv = AdoFldBool(flds, "PrivEmail", false);

		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryPatientInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;
		CString strLine;

		//add the header row
		AddListDataToArray(paryPatientInfo, "Patient Information", RGB(0,0,192), "tabG1");

		//First Middle Last
		if (strMiddle.IsEmpty()) {
			strLine = strFirst + " " + strLast;
		}
		else {
			strLine = strFirst + " " + strMiddle + " " + strLast;
		}
		if (!strPreFix.IsEmpty()) {
			strLine = strPreFix + " " + strLine;
		}
		if  (!strTitle.IsEmpty()) {
			strLine += ", " + strTitle;
		}
		AddListDataToArray(paryPatientInfo, strLine);

		//Address1
		if (!strAdd1.IsEmpty()) {
			AddListDataToArray(paryPatientInfo, strAdd1);		
		}

		//Address2
		if (!strAdd2.IsEmpty()) {
			AddListDataToArray(paryPatientInfo, strAdd2);
		}

		//City, State Zip Country
		if (strCountry.IsEmpty()) {
			strLine = strCity + ", " + strState + " " + strZip;
		}
		else {
			strLine = strCity + ", " + strState + " " + strZip + " " + strCountry; 
		}
		AddListDataToArray(paryPatientInfo, strLine);

		//NickName		
		if (!strNickName.IsEmpty()) {
			AddListDataToArray(paryPatientInfo, "Dear: " + strNickName);
		}

		//SSN
		strSSN.Trim();
		if (!strSSN.IsEmpty()) {
			AddListDataToArray(paryPatientInfo, "SSN: " + strSSN);
		}

		//BirthDate (Age)
		if (dtBirthDate.GetStatus() != COleDateTime::invalid) {
			CString strBDay = FormatDateTimeForInterface(dtBirthDate, NULL, dtoDate);
			if (strBDay.CompareNoCase("Invalid DateTime") != 0) {
				strLine.Format("Birth Date: %s (%s)", strBDay, strAge);
				AddListDataToArray(paryPatientInfo, strLine);
			}
		}

		//Marital Status - SpouseName
		CString strMS;
		switch (nMaritalStatus) {
			case 1: //single
				strMS = "Single";
			break;
			case 2: //married
				strMS = "Married";
			break;
			case 3:	//other
				strMS = "Other";
			break;
			default:
				"";
			break;
		}
		if (!strMS.IsEmpty())  {
			
			if (strSpouseName.IsEmpty()) {
				strLine = "Marital Status: " + strMS;
			}
			else {
				strLine = "Marital Status: " + strMS + " - " + strSpouseName;
			}

			AddListDataToArray(paryPatientInfo, strLine);
		}				

		//Gender
		if (nGender == 1) {
			AddListDataToArray(paryPatientInfo, "Gender: Male");
		}
		else if (nGender == 2) {
			AddListDataToArray(paryPatientInfo, "Gender: Female");
		}
		
		//HomePhone
		if (!strHome.IsEmpty()) {
			if (bHomePriv) {
				strLine = "Home: " + strHome + " (Private)";
			}
			else {
				strLine = "Home: " + strHome;
			}
			AddListDataToArray(paryPatientInfo, strLine);
		}
			
		//WorkPhone ext:Ext
		if (!strWorkPhone.IsEmpty()) {
			if (!strExt.IsEmpty()) {
				strLine = "Work: " + strWorkPhone + " ext: " + strExt;
			}
			else {
				strLine = "Work: " + strWorkPhone;
			}
			if (bWorkPriv) {
				strLine = strLine + " (Private)";
			}				
			AddListDataToArray(paryPatientInfo, strLine);
		}

		//CellPhone		
		if (!strCellPhone.IsEmpty()) {
			if (bCellPriv) {
				strLine = "Cell: " + strCellPhone + " (Private)";
			}
			else {
				strLine = "Cell: " + strCellPhone;
			}
			AddListDataToArray(paryPatientInfo, strLine);
		}

		//Pager
		if (!strPager.IsEmpty()) {
			if (bPagerPriv) {
				strLine = "Pager: " + strPager + " (Private)";
			}
			else {
				strLine = "Pager: " + strPager;
			}
			AddListDataToArray(paryPatientInfo, strLine);
		}

		//Other
		if (!strOther.IsEmpty()) {
			if (bOtherPriv) {
				strLine = "Other: " + strOther + " (Private)";
			}
			else {
				strLine = "Other: " + strOther;
			}
			AddListDataToArray(paryPatientInfo, strLine);
		}

		//Fax
		if (!strFax.IsEmpty()) {
			if (bFaxPriv) {
				strLine = "Fax: " + strFax + " (Private)";
			}
			else {
				strLine = "Fax: " + strFax;
			}
			AddListDataToArray(paryPatientInfo, strLine);
		}

		//Email
		if (bDeclinedEmail) {
			strEmail = "<Declined>";
		}
		if (!strEmail.IsEmpty()) {			
			if (bEmailPriv) {
				strLine = "Email: " + strEmail + " (Private)";
			}
			else {
				strLine = "Email: " + strEmail;
			}
			AddListDataToArray(paryPatientInfo, strLine);
		}

		//PrefContact
		CString strPC;
		switch(nPrefContact) {			
			
			case 1:
				strPC =  "Home Phone";
				break;
			case 2:
				strPC =  "Work Phone";
				break;
			case 3:
				strPC =  "Mobile Phone";
				break;
			case 4:
				strPC =  "Pager";
				break;
			case 5:
				strPC =  "Other Phone";
				break;
			case 6:
				strPC =  "Email";
				break;
			case 7:
				strPC =  "Text Messaging";
				break;
			default:				
			break;
		}
		if (!strPC.IsEmpty()) {
			AddListDataToArray(paryPatientInfo, "Preferred Contact: " + strPC);
		}

		//now we have to add a blank line
		AddListDataToArray(paryPatientInfo);

		//now add to the list
		CPatSummaryConfigType cfgType = GetConfigInfo("PatSumPatInfo");
		if (cfgType.m_nList == 1) {
			//put it in the left list	
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryPatientInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryPatientInfo);
			m_aryRightList.Add(paryList);
		}
	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayEmergencyContactInfo(ADODB::_RecordsetPtr rsAllRecs)
{
	if (!rsAllRecs->eof) {
		
		//format
		//Emergency Contact
		//strFirst strLast
		//Relationship:
		//Home:	
		//Work:
		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryPatientInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;

		CString strFirst = AdoFldString(rsAllRecs, "EmergFirst", "");
		CString strLast = AdoFldString(rsAllRecs, "EmergLast", "");
		CString strRelation = AdoFldString(rsAllRecs, "EmergRelation", "");
		CString strHome = AdoFldString(rsAllRecs, "EmergHPhone", "");
		CString strWork = AdoFldString(rsAllRecs, "EmergWPhone", "");

		//add the header row
		AddListDataToArray(paryPatientInfo, "Emergency Contact", RGB(0,0,192), "tabG1");

		//strFirst strLast
		AddListDataToArray(paryPatientInfo, strFirst + " " + strLast);
		//Relationship:
		if (!strRelation.IsEmpty()){
			AddListDataToArray(paryPatientInfo, "Relationship: " + strRelation);
		}
		//Home:	
		if (!strHome.IsEmpty()){
			AddListDataToArray(paryPatientInfo, "Home: " + strHome);
		}
		//Work
		if (!strWork.IsEmpty()){
			AddListDataToArray(paryPatientInfo, "Work: " + strWork);
		}

		//now we have to add a blank line
		AddListDataToArray(paryPatientInfo);

		//now add to the list
		CPatSummaryConfigType cfgType = GetConfigInfo("PatSumEmergencyContactInfo");
		if (cfgType.m_nList == 1) {
			//put it in the left list				
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryPatientInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryPatientInfo);
			m_aryRightList.Add(paryList);
		}
	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayLanguageField(ADODB::_RecordsetPtr rsAllRecs)
{

	CPatSummaryConfigType cfgType = GetConfigInfo("PatSumLanguage");
	if (cfgType.m_bCheck) {

		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryLangInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;

		//add the header row
		AddListDataToArray(paryLangInfo, "Language", RGB(0,0,192), "tabG2");

		if (rsAllRecs->eof) {
			AddListDataToArray(paryLangInfo, "<None>");
		}

		while (!rsAllRecs->eof) {

			CString strField = AdoFldString(rsAllRecs, "Field", "");			

			if (strField.IsEmpty()) {
				AddListDataToArray(paryLangInfo, "<No Language Selected>");
			}
			else {

				// (d.thompson 2012-08-16) - PLID 52046 - We moved the name/code pairings into data a while ago, and
				//	apparently noone ever fixed this.  I've updated the query so it now pulls the name from data, and
				//	deleted the name lookup function.
				AddListDataToArray(paryLangInfo, strField);
			}
			rsAllRecs->MoveNext();
		}

		//now we have to add a blank line
		AddListDataToArray(paryLangInfo);

		//now add to the list
		if (cfgType.m_nList == 1) {
			//put it in the left list	
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryLangInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryLangInfo);
			m_aryRightList.Add(paryList);
		}
	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayGeneralField(ADODB::_RecordsetPtr rsAllRecs, CString strTitle, CString strTab, CString strBase)
{

	CPatSummaryConfigType cfgType = GetConfigInfo(strBase);
	if (cfgType.m_bCheck) {

		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryGenInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;

		//add the header row
		AddListDataToArray(paryGenInfo, strTitle, RGB(0,0,192), strTab);

		if (rsAllRecs->eof) {
			AddListDataToArray(paryGenInfo, "<None>");
		}
		// (s.tullis 2014-03-06 10:00) - PLID 60762 - 40090 - Patients Module > NexEMR > Pt. Summary > Default Diagnosis Codes (in top ½)
		if ( strBase.Compare("PatSumDiagCodes")== 0)
		{
			while(!rsAllRecs->eof)
			{
				
				CString DiagICD9 = AdoFldString(rsAllRecs, "DefaultICD9", "");
				CString DiagICD10= AdoFldString(rsAllRecs, "DefaultICD10", "");
				CString Index = AdoFldString(rsAllRecs, "IndexNumber", "");
				
				
				// both empty
				if( DiagICD9.IsEmpty() &&  DiagICD10.IsEmpty())
				{
					
						AddListDataToArray(paryGenInfo, "<None>");

				}// ICD9 and no ICD10
				else if( DiagICD9.IsEmpty() &&  !(DiagICD10.IsEmpty()))
				{

						DiagICD10= Index +". "+ DiagICD10;	
						AddListDataToArray(paryGenInfo, DiagICD10);
						
				}
				// ICD10 and no ICD9
				else if ( !(DiagICD9.IsEmpty()) &&  DiagICD10.IsEmpty())
				{	
						DiagICD9= Index +". "+ DiagICD9;
						AddListDataToArray(paryGenInfo, DiagICD9);
				}//Both ICD10 and ICD9
				else if( !(DiagICD9.IsEmpty()) &&  !(DiagICD10.IsEmpty()))
				{	

						DiagICD9= Index +". "+ DiagICD9;
						DiagICD10="    "+DiagICD10;
						AddListDataToArray(paryGenInfo, DiagICD9);
						AddListDataToArray(paryGenInfo, DiagICD10);
					
				}
				rsAllRecs->MoveNext();
				


			}

		
		}
		else{	
			while (!rsAllRecs->eof) {

				CString strField = AdoFldString(rsAllRecs, "Field", "");			

				if (strField.IsEmpty()) {
					AddListDataToArray(paryGenInfo, "<None>");
				}
				else {
					AddListDataToArray(paryGenInfo, strField);
				}
				rsAllRecs->MoveNext();
			}
		}
		//now we have to add a blank line
		AddListDataToArray(paryGenInfo);

		//now add to the list
		if (cfgType.m_nList == 1) {
			//put it in the left list	
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryGenInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryGenInfo);
			m_aryRightList.Add(paryList);
		}
	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayGeneralDate(ADODB::_RecordsetPtr rsAllRecs, CString strTitle, CString strTab, CString strBase, BOOL bDisplayTime /*=FALSE*/)
{
	CPatSummaryConfigType cfgType = GetConfigInfo(strBase);
	if (cfgType.m_bCheck) {

		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryGenInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;

		//add the header row
		AddListDataToArray(paryGenInfo, strTitle, RGB(0,0,192), strTab);
	
		if (rsAllRecs->eof) {
			AddListDataToArray(paryGenInfo, "<None>");
		}
	
		while (!rsAllRecs->eof) {

			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			COleDateTime dt = AdoFldDateTime(rsAllRecs, "Date", dtInvalid);

			CString strDate;
			if (bDisplayTime) {
				strDate = FormatDateTimeForInterface(dt, NULL, dtoDateTime);
			}
			else {
				strDate = FormatDateTimeForInterface(dt, NULL, dtoDate);
			}

			if ((dt.GetStatus() == COleDateTime::invalid) || strDate.CompareNoCase("Invalid DateTime") == 0) {
				AddListDataToArray(paryGenInfo, "<None>");
			}
			else {				
				AddListDataToArray(paryGenInfo, strDate);
			}
			rsAllRecs->MoveNext();
		}

		//now we have to add a blank line
		AddListDataToArray(paryGenInfo);

		//now add to the list		
		if (cfgType.m_nList == 1) {
			//put it in the left list	
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryGenInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryGenInfo);
			m_aryRightList.Add(paryList);
		}
	}

}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayEmploymentInfo(ADODB::_RecordsetPtr rsAllRecs)
{

	CPatSummaryConfigType cfgType = GetConfigInfo("PatSumEmploymentInfo");
	if (cfgType.m_bCheck) {

		//Format:
		//Employment Information
		//Status
		//Occupation
		//Company
		//Manager First, Middle, Last
		//Address
		//Address2
		//City, State, Zip
		FieldsPtr flds = rsAllRecs->Fields;

		//Employment Information
		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryEmpInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;

		//add the header row
		AddListDataToArray(paryEmpInfo, "Employment Information", RGB(0,0,192), "tabG2");


		if (!rsAllRecs->eof) {

			CString strOccupation = AdoFldString(flds, "Occupation", "");
			long nStatus = AdoFldLong(flds, "Employment", -1);
			CString strCompany  = AdoFldString(flds, "Company", "");
			CString strFirst = AdoFldString(flds, "EmployerFirst", "");
			CString strMiddle = AdoFldString(flds, "EmployerMiddle", "");
			CString strLast = AdoFldString(flds, "EmployerLast", "");
			CString strAdd1 = AdoFldString(flds, "EmployerAddress1", "");
			CString strAdd2 = AdoFldString(flds, "EmployerAddress2", "");
			CString strCity = AdoFldString(flds, "EmployerCity", "");
			CString strState = AdoFldString(flds, "EmployerState", "");
			CString strZip = AdoFldString(flds, "EmployerZip", "");

			//Status
			CString strStatus;
			switch (nStatus) {
				case 1:
					strStatus = "Full-Time";
				break;
				
				case 2:			
					strStatus = "Full-Time Student";
				break;
				case 3:
					strStatus = "Part-Time Student";
				break;
				case 4:
					strStatus = "Part-Time";
				break;
				case 5:
					strStatus = "Retired";
				break;
				case 6:			
					strStatus = "Other";
				break;
				default:
					strStatus = "Other";
				break;
			}
			if (!strStatus.IsEmpty()) {
				AddListDataToArray(paryEmpInfo, "Status: " + strStatus);
			}

			//Occupation
			if (!strOccupation.IsEmpty()) {
				AddListDataToArray(paryEmpInfo, "Occupation: " + strOccupation);
			}

			//Company
			if (!strOccupation.IsEmpty()) {
				AddListDataToArray(paryEmpInfo, "Company: " + strCompany);
			}
			//Manager First, Middle, Last
			CString strLine;
			if (strMiddle.IsEmpty()) {
				strLine = strFirst + " " + strLast;
			}
			else {
				strLine = strFirst + " " + strMiddle + " " + strLast;
			}
			strLine.TrimRight();
			strLine.TrimLeft();
			if (!strLine.IsEmpty()) {
				AddListDataToArray(paryEmpInfo, "Manager: " + strLine);
			}

			//Address
			if (!strAdd1.IsEmpty()) {
				AddListDataToArray(paryEmpInfo, strAdd1);
			}
			//Address2
			if (!strAdd2.IsEmpty()) {
				AddListDataToArray(paryEmpInfo, strAdd2);
			}
			//City, State, Zip
			strLine = strCity + ", " + strState + " " + strZip;
			strLine.TrimLeft();
			strLine.TrimRight();
			if (strLine != ",") {
				AddListDataToArray(paryEmpInfo, strLine);
			}
		}

		//now we have to add a blank line
		AddListDataToArray(paryEmpInfo);

		//now add to the list		
		if (cfgType.m_nList == 1) {
			//put it in the left list		
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryEmpInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryEmpInfo);
			m_aryRightList.Add(paryList);
		}

	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayReferralInformation(ADODB::_RecordsetPtr rsAllRecs)
{
	CPatSummaryConfigType cfgType = GetConfigInfo("PatSumRefInfo");
	if (cfgType.m_bCheck) {

		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryRefInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;

		//add the header row
		AddListDataToArray(paryRefInfo, "Referring Information", RGB(0,0,192), "tabG2");

		if (rsAllRecs->eof) {
			AddListDataToArray(paryRefInfo, "<None>");
		}

		while (!rsAllRecs->eof) {

			CString strField = AdoFldString(rsAllRecs, "Name", "");
			long nType = AdoFldLong(rsAllRecs,"Type", -1);

			if (!strField.IsEmpty()) {
				if (nType == 1) {
					AddListDataToArray(paryRefInfo, strField, RGB(255,0,0));
				}
				else if (nType == 3) {
					AddListDataToArray(paryRefInfo, "Referring Physician: " + strField);
				}
				else if (nType == 4) {
					AddListDataToArray(paryRefInfo, "Referring Patient: " + strField);
				}
				else if (nType == 5) {
					AddListDataToArray(paryRefInfo, "PCP: " + strField);
				}
				else {
					AddListDataToArray(paryRefInfo, strField);
				}
			}
			rsAllRecs->MoveNext();
		}

		//now we have to add a blank line
		AddListDataToArray(paryRefInfo);

		//now add to the list		
		if (cfgType.m_nList == 1) {
			//put it in the left list
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryRefInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryRefInfo);
			m_aryRightList.Add(paryList);
		}
	}

}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayWarningInformation(ADODB::_RecordsetPtr rsAllRecs)
{
	CPatSummaryConfigType cfgType = GetConfigInfo("PatSumWarning");
	if (cfgType.m_bCheck) {

		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryWarnInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;

		//add the header row
		AddListDataToArray(paryWarnInfo, "Patient Warning", RGB(0,0,192), "tabG2");

		if (rsAllRecs->eof) {
			AddListDataToArray(paryWarnInfo, "<None>");
		}

		if (!rsAllRecs->eof) {
			COleDateTime dtInvalid;
			dtInvalid.SetStatus(COleDateTime::invalid);
			CString strWarning = AdoFldString(rsAllRecs, "WarningMessage", "");
			BOOL bDisplay = AdoFldBool(rsAllRecs, "DisplayWarning", false);
			BOOL bExpires = AdoFldBool(rsAllRecs, "WarningUseExpireDate", false);
			COleDateTime dtExpire = AdoFldDateTime(rsAllRecs, "WarningExpireDate", dtInvalid);
			CString strUser = AdoFldString(rsAllRecs, "UserName", "");

			if (strWarning.IsEmpty()) {
				AddListDataToArray(paryWarnInfo, "<None>");
			}
			else if (!bDisplay) {
				AddListDataToArray(paryWarnInfo, "<Not Displayed>");
			}
			else {
				AddListDataToArray(paryWarnInfo, strWarning);

				if (bExpires) {
					//check the date
					if (dtExpire.GetStatus() != COleDateTime::invalid) {
						AddListDataToArray(paryWarnInfo, "Expires on: " + FormatDateTimeForSql(dtExpire, dtoDate));
					}			
				}
			}
			
			
		}

		//now we have to add a blank line
		AddListDataToArray(paryWarnInfo);

		//now add to the list		
		if (cfgType.m_nList == 1) {
			//put it in the left list	
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryWarnInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryWarnInfo);
			m_aryRightList.Add(paryList);
		}
	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayInsPartyInfo(ADODB::_RecordsetPtr rsAllRecs)
{
	CPatSummaryConfigType cfgType = GetConfigInfo("PatSumInsPartyInfo");
	if (cfgType.m_bCheck) {

		//Format is:
		//InsCoName: <Type>
		//First Middle Last
		//Address1
		//Address2
		//City, State Zip
		//Company
		//BirthDate
		//SSN
		//Phone
		//RelationToPat
		//Gender
		//PlanName - PlanType
		//IDForInsurance
		//PolicyGroup		
		//Copay
		
		COleDateTime dtInvalid;
		dtInvalid.SetStatus(COleDateTime::invalid);

		FieldsPtr flds = rsAllRecs->Fields;

		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryInsPartyInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;
		
		//add the header row
		AddListDataToArray(paryInsPartyInfo, "Insurance Information", RGB(0,0,192), "tabInsurance");

		if (rsAllRecs->eof) {
			AddListDataToArray(paryInsPartyInfo, "<None>");
			
			//now we have to add a blank line
			AddListDataToArray(paryInsPartyInfo);
		}

		while (!rsAllRecs->eof) {

			CString strFirst = AdoFldString(flds, "First", "");
			CString strMiddle = AdoFldString(flds, "Middle", "");
			CString strLast = AdoFldString(flds, "Last", "");
			CString strAdd1 = AdoFldString(flds, "Address1", "");
			CString strAdd2 = AdoFldString(flds, "Address2", "");
			CString strCity = AdoFldString(flds, "City", "");
			CString strState = AdoFldString(flds, "State", "");
			CString strZip = AdoFldString(flds, "Zip", "");		
			CString strSSN = AdoFldString(flds, "SocialSecurity", "").Trim();
			COleDateTime dtBirthDate = AdoFldDateTime(flds, "Birthdate", dtInvalid);		
			long nGender = AdoFldByte(flds, "Gender", 0);
			CString strHome = AdoFldString(flds, "HomePhone", "");
			CString strRelation = AdoFldString(flds, "RelationtoPatient", "");
			CString strPlanName = AdoFldString(flds, "InsPlan", "");
			CString strPlanType = AdoFldString(flds, "PlanType", "");
			CString IDForIns = AdoFldString(flds, "IDForInsurance", "");
			CString strPolicy = AdoFldString(flds, "PolicyGroupNum", "");
			CString strPriority = AdoFldString(flds, "TypeName", "");
			CString strInsCoName = AdoFldString(flds, "CompanyName", "");
			CString strCompany = AdoFldString(flds, "Company", "");			
			
			CString strLine;		
			
			// (f.dinatale 2011-02-16) - PLID 42260 - Added permission checking to mask the Insured Party SSN for the Patient Summary dialog.
			if(CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
				strSSN = FormatSSNText(strSSN, eSSNNoMask, "###-##-####");
			} else {
				if(CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && !CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
					strSSN = FormatSSNText(strSSN, eSSNPartialMask, "###-##-####");
				} else {
					strSSN = "";
				}
			}

			//InsCoName: Type
			strLine.Format("%s: %s", strPriority, strInsCoName);
			AddListDataToArray(paryInsPartyInfo, strLine);

			//First Middle Last
			if (strMiddle.IsEmpty()) {
				strLine = strFirst + " " + strLast;
			}
			else {
				strLine = strFirst + " " + strMiddle + " " + strLast;
			}
			AddListDataToArray(paryInsPartyInfo, strLine);

			//Address1
			AddListDataToArray(paryInsPartyInfo, strAdd1);		

			//Address2
			if (!strAdd2.IsEmpty()) {
				AddListDataToArray(paryInsPartyInfo, strAdd2);
			}

			//City, State Zip 
			strLine = strCity + ", " + strState + " " + strZip;
			AddListDataToArray(paryInsPartyInfo, strLine);

			//Company		
			if (!strCompany.IsEmpty()) {
				AddListDataToArray(paryInsPartyInfo, strCompany);
			}

			//BirthDate
			if (dtBirthDate.GetStatus() != COleDateTime::invalid) {
				CString strBirthDate = FormatDateTimeForInterface(dtBirthDate, NULL, dtoDate);
				if (strBirthDate.CompareNoCase("Invalid DateTime") != 0) {
					strLine.Format("Birth Date: %s ", strBirthDate);
					AddListDataToArray(paryInsPartyInfo, strLine);
				}
			}

			//SSN
			if (!strSSN.IsEmpty()) {
				AddListDataToArray(paryInsPartyInfo, "SSN: " + strSSN);
			}	

			//Phone
			if (!strHome.IsEmpty()) {
				strLine = "Home: " + strHome;				
				AddListDataToArray(paryInsPartyInfo, strLine);
			}		

			//Relation To Patient
			AddListDataToArray(paryInsPartyInfo, "Relation: "+ strRelation);

			//Gender
			if (nGender == 1) {
				AddListDataToArray(paryInsPartyInfo, "Gender: Male");
			}
			else if (nGender == 2) {
				AddListDataToArray(paryInsPartyInfo, "Gender: Female");
			}
			
			//PlanName - Type
			if (!strPlanName.IsEmpty()) {
				strLine = "Plan: " + strPlanName;
				if (!strPlanType.IsEmpty()) {
					strLine += " - " + strPlanType;
				}
				AddListDataToArray(paryInsPartyInfo, strLine);
			}
				
			//ID For Insurance
			if (!IDForIns.IsEmpty()) {
				strLine = "ID For Insurance: " + IDForIns;				
				AddListDataToArray(paryInsPartyInfo, strLine);
			}	

			//Policy Group Num
			if (!strPolicy.IsEmpty()) {
				strLine = "Policy Group or FECA Number: " + strPolicy;				
				AddListDataToArray(paryInsPartyInfo, strLine);
			}			

			//Copay
			// (j.gruber 2010-08-03 15:38) - PLID 39948 - change the structure
			_variant_t varCopay, varPercent;
			varCopay = rsAllRecs->Fields->Item["Copay"]->Value;
			varPercent = rsAllRecs->Fields->Item["CopayPercent"]->Value;
			if (varCopay.vt == VT_CY) {
				CString strCopay;
				strCopay.Format("Copay: %s",FormatCurrencyForInterface(AdoFldCurrency(rsAllRecs, "CoPay",COleCurrency(0,0))));
				AddListDataToArray(paryInsPartyInfo, strCopay);
			}
			else if (varPercent.vt == VT_I4)
			{
				//percentage
				CString strCopay;
				strCopay.Format("Copay: %li%%",AdoFldLong(rsAllRecs, "CopayPercent",0));
				AddListDataToArray(paryInsPartyInfo, strCopay);
			}			

			//now we have to add a blank line
			AddListDataToArray(paryInsPartyInfo);

			rsAllRecs->MoveNext();
		}

		//now add to the list		
		if (cfgType.m_nList == 1) {
			//put it in the left list	
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryInsPartyInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryInsPartyInfo);
			m_aryRightList.Add(paryList);
		}
	}
}


// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayCurrentMeds(ADODB::_RecordsetPtr rsAllRecs)
{
	CPatSummaryConfigType cfgType = GetConfigInfo("PatSumCurMeds");
	if (cfgType.m_bCheck) {

		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryMedInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;

		//add the header row
		AddListDataToArray(paryMedInfo, "Current Medications", RGB(0,0,192), "tabMed");

		if (rsAllRecs->eof) {
			AddListDataToArray(paryMedInfo, "<None>");
		}

		while (!rsAllRecs->eof) {

			CString strMedName = AdoFldString(rsAllRecs, "Data", "");
			BOOL bDiscontinued = AdoFldBool(rsAllRecs, "Discontinued", false);

			if (bDiscontinued) {
				AddListDataToArray(paryMedInfo, strMedName + " (Inactive)");
			}
			else {
				AddListDataToArray(paryMedInfo, strMedName);
			}
			
			rsAllRecs->MoveNext();
		}

		//now we have to add a blank line
		AddListDataToArray(paryMedInfo);

		//now add to the list		
		if (cfgType.m_nList == 1) {
			//put it in the left list	
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryMedInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryMedInfo);
			m_aryRightList.Add(paryList);
		}
	}

}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayAllergies(ADODB::_RecordsetPtr rsAllRecs)
{
	CPatSummaryConfigType cfgType = GetConfigInfo("PatSumAllergies");
	if (cfgType.m_bCheck) {

		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryMedInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;

		//add the header row
		AddListDataToArray(paryMedInfo, "Allergies", RGB(0,0,192), "tabMed");

		if (rsAllRecs->eof) {
			AddListDataToArray(paryMedInfo, "<None>");
		}

		while (!rsAllRecs->eof) {

			CString strMedName = AdoFldString(rsAllRecs, "Data", "");
			BOOL bDiscontinued = AdoFldBool(rsAllRecs, "Discontinued", false);

			if (bDiscontinued) {
				AddListDataToArray(paryMedInfo, strMedName + " (Inactive)");
			}
			else {
				AddListDataToArray(paryMedInfo, strMedName);
			}
			
			rsAllRecs->MoveNext();
		}

		//now we have to add a blank line
		AddListDataToArray(paryMedInfo);

		//now add to the list		
		if (cfgType.m_nList == 1) {
			//put it in the left list			
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryMedInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryMedInfo);
			m_aryRightList.Add(paryList);
		}
	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayLastAppt(ADODB::_RecordsetPtr rsAllRecs)
{
	CPatSummaryConfigType cfgType = GetConfigInfo("PatSumLastApptInfo");
	if (cfgType.m_bCheck) {

		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryApptInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;
		
		AddListDataToArray(paryApptInfo, "Last Appointment Information", RGB(0,0,192), "tabAppt");

		BOOL bCanSeeAppointments = TRUE;
		CString strLastAppointment = "<Not Available>";

		//check licensing
		if(!g_pLicense || !g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent)) {
			bCanSeeAppointments = FALSE;
			strLastAppointment = "<Not Available>";
		}

		//make sure they have unrestricted view permission
		if(bCanSeeAppointments && !(GetCurrentUserPermissions(bioAppointment) & sptView)) {
			bCanSeeAppointments = FALSE;
			strLastAppointment = "<Not Available>";
		}		

		if(bCanSeeAppointments) {

			if(!rsAllRecs->eof) {

				COleDateTime dtTime = AdoFldDateTime(rsAllRecs, "StartTime");
				CString strResource = AdoFldString(rsAllRecs, "Resource", "");
				CString strPurpose = AdoFldString(rsAllRecs, "Purpose", "");

				if(strPurpose.IsEmpty()) {
					strPurpose = "<No Type, No Purpose>";
				}

				strLastAppointment.Format("%s - %s, Resource: %s", FormatDateTimeForInterface(dtTime, DTF_STRIP_SECONDS, dtoDateTime), strPurpose, strResource);
			}
			else {
				strLastAppointment = "<No Previous Appointment>";
			}		
		}

		AddListDataToArray(paryApptInfo, strLastAppointment);

		//add a blank row
		AddListDataToArray(paryApptInfo);

		//now add to the list		
		if (cfgType.m_nList == 1) {
			//put it in the left list			
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryApptInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryApptInfo);
			m_aryRightList.Add(paryList);
		}
	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen - modified slightly from old code to fit new structure
void CPatientSummaryDlg::DisplayLastDiagCodesBilled(ADODB::_RecordsetPtr rsAllRecs)
{
	CPatSummaryConfigType cfgType = GetConfigInfo("PatSumLastDiagsBilled");
	if (cfgType.m_bCheck) {
		
		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryDiagInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;
		
		AddListDataToArray(paryDiagInfo, "Last Diagnosis Codes Billed", RGB(0,0,192), "tabBill");

		BOOL bCanSeeBills = TRUE;

		//check billing licensing & perms
		if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
			bCanSeeBills = FALSE;
		}

		//make sure they have unrestricted read permission
		if(bCanSeeBills && !(GetCurrentUserPermissions(bioPatientBilling) & sptRead)) {
			bCanSeeBills = FALSE;
		}
		
		CString strLastBilledDiagCodes = "";
		CString strDiagCodes= "";
		
		if(bCanSeeBills) {

			if(rsAllRecs->eof) {
				strLastBilledDiagCodes = "<No Previous Bill>";
				
				AddListDataToArray(paryDiagInfo, strLastBilledDiagCodes);
				AddListDataToArray(paryDiagInfo);
			}
			else {
					
					int i;
					long BillID;
					bool bDiagICD9=false;
					bool bDiagICD10=false;
					CString strICD10="";
					CString strICD9="";
					CStringArray arrICD9;
					CStringArray arrICD10;



					
				// (s.tullis 2014-03-07 08:40) - PLID 60761 - 40080 - Patients Module > NexEMR > Pt. Summary > Last Diagnosis Codes Billed (in top ½)
					// see if the records contain Only ICD9 , Only ICD10 , or Mix of Both because it will affect how the diagnosis codes are diplayed
					while(!(rsAllRecs->eof))
					{
						
						BillID= AdoFldLong(rsAllRecs,"ID",-1);
						strICD9=AdoFldString(rsAllRecs,"ICD9Code","");
						strICD10=AdoFldString(rsAllRecs,"ICD10Code","");

						// No previous Bill
						if(BillID ==-1){
							continue;	
						}
						
						if(!(strICD9.IsEmpty())){
							arrICD9.Add(strICD9);
							bDiagICD9=true;
						}
						else
						{
							arrICD9.Add(strICD9);
						}

						if(!(strICD10.IsEmpty())){
							arrICD10.Add(strICD10);
							bDiagICD10=true;
						}
						else{
							arrICD10.Add(strICD10);
							
						}

							rsAllRecs->MoveNext();
					}
										
								//check all ICD10's and no ICD9
					if (   bDiagICD10==true  &&  bDiagICD9==false)
						{	
								
								for(i=0 ; i< arrICD10.GetSize(); i++)
								{
									if( i == arrICD10.GetSize()-1){
										strLastBilledDiagCodes=strLastBilledDiagCodes+arrICD10[i];
									}
									else{
										strLastBilledDiagCodes=strLastBilledDiagCodes+arrICD10[i]+",";				
									}
								
								}

								AddListDataToArray(paryDiagInfo, strLastBilledDiagCodes);

								AddListDataToArray(paryDiagInfo);

						}// Check all ICD9's and no ICD10
						else if ( bDiagICD9==true &&  bDiagICD10==false)
									{	
										
										for(i=0 ; i< arrICD9.GetSize(); i++)
										{
											if( i == arrICD9.GetSize()-1){
												strLastBilledDiagCodes=strLastBilledDiagCodes+arrICD9[i];
											}
											else{
												strLastBilledDiagCodes=strLastBilledDiagCodes+arrICD9[i]+",";
											}
										}

										AddListDataToArray(paryDiagInfo, strLastBilledDiagCodes);

										AddListDataToArray(paryDiagInfo);
								
								   }

								 // mix of of them both ICD9 and ICD10
						else if (bDiagICD9==true &&  bDiagICD10==true)
								{
									{	
										
										if( arrICD9.GetSize()==arrICD10.GetSize())
										{
											for(i=0 ; i< arrICD9.GetSize(); i++)
											{
												if (!(arrICD9[i].IsEmpty()) && !(arrICD10[i].IsEmpty())  ){
													strLastBilledDiagCodes=arrICD9[i]+" / "+arrICD10[i];

												}
												else if(arrICD9[i].IsEmpty()){
													strLastBilledDiagCodes=arrICD10[i];
												}
												else if(arrICD10[i].IsEmpty()){
													strLastBilledDiagCodes=arrICD9[i];
												}



												AddListDataToArray(paryDiagInfo, strLastBilledDiagCodes);
											

											}

											AddListDataToArray(paryDiagInfo);
										}
								
									}
										// no diag codes on last bill
						}else if ( 	bDiagICD9==false && bDiagICD10==false){

									strLastBilledDiagCodes = "<No Diagnosis Codes On Last Bill>";
									
									AddListDataToArray(paryDiagInfo, strLastBilledDiagCodes);
											
									AddListDataToArray(paryDiagInfo);


						}
							
				}
		}else{//default <not available>
			strLastBilledDiagCodes ="<Not Available>";
			AddListDataToArray(paryDiagInfo, strLastBilledDiagCodes);
			AddListDataToArray(paryDiagInfo);
		}
								

		
		
		

		//now add to the list		
		if (cfgType.m_nList == 1) {
			//put it in the left list			
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryDiagInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryDiagInfo);
			m_aryRightList.Add(paryList);
		}
	}		
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DisplayGeneralCurrency(ADODB::_RecordsetPtr rsAllRecs, CString strTitle, CString strTab, CString strBase)
{
	CPatSummaryConfigType cfgType = GetConfigInfo(strBase);
	if (cfgType.m_bCheck) {

		CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryGenInfo = new CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*>;

		//add the header row
		AddListDataToArray(paryGenInfo, strTitle, RGB(0,0,192), strTab);

		BOOL bCanSeeBills = TRUE;	
		//check billing licensing & perms
		if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
			bCanSeeBills = FALSE;
		}

		//make sure they have unrestricted read permission
		if(bCanSeeBills && !(GetCurrentUserPermissions(bioPatientBilling) & sptRead)) {
			bCanSeeBills = FALSE;
		}
		
		CString strCurrency = "<Not Available>";
	
		if (bCanSeeBills) {
		
			COleCurrency cy = AdoFldCurrency(rsAllRecs, "Currency", COleCurrency(0,0));
			strCurrency = FormatCurrencyForInterface(cy);		
		}
		AddListDataToArray(paryGenInfo, strCurrency);

		//now we have to add a blank line
		AddListDataToArray(paryGenInfo);

		//now add to the list
		
		if (cfgType.m_nList == 1) {
			//put it in the left list	
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryGenInfo);
			m_aryLeftList.Add(paryList);
		}
		else {
			CPatSummaryList *paryList = new CPatSummaryList(cfgType.m_nSortOrder, paryGenInfo);
			m_aryRightList.Add(paryList);
		}
	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
static int CompareSortOrder(const void *pa, const void *pb)
{
	CPatSummaryList **ppls1 = (CPatSummaryList**)pa;
	CPatSummaryList **ppls2 = (CPatSummaryList**)pb;

	if (*ppls1 != NULL && &ppls2 != NULL) {
		if ((*ppls1)->m_nSortOrder < (*ppls2)->m_nSortOrder) {
			return -1;
		} else if ((*ppls1)->m_nSortOrder > (*ppls2)->m_nSortOrder) {
			return 1;
		} else {
			return 0;
		}
	}
	else {
		ASSERT(FALSE);
		return -1;
	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::SortArray(CArray<CPatSummaryList*, CPatSummaryList*> *pList)
{
	qsort(m_aryLeftList.GetData(), m_aryLeftList.GetSize(), sizeof(CPatSummaryList*), CompareSortOrder);

	qsort(m_aryRightList.GetData(), m_aryRightList.GetSize(), sizeof(CPatSummaryList*), CompareSortOrder);

}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::AddArraysToList()
{
	//find the number of rows we'll need

	long nLeftCount = 0;
	long nRightCount = 0;

	for (int i = 0; i < m_aryLeftList.GetSize(); i++) {
		CPatSummaryList *pItem = m_aryLeftList.GetAt(i);

		if (pItem) {
			nLeftCount += pItem->m_paryObjects->GetSize();
		}
	}

	for (int i = 0; i < m_aryRightList.GetSize(); i++) {
		CPatSummaryList *pItem = m_aryRightList.GetAt(i);

		if (pItem) {
			nRightCount += pItem->m_paryObjects->GetSize();
		}
	}

	long nRowsNeeded = 0;
	if (nLeftCount <= nRightCount) {
		nRowsNeeded = nRightCount;
	}
	else {
		nRowsNeeded = nLeftCount;
	}

	for (int i = 0; i < nRowsNeeded; i++) {

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetNewRow();

		pRow->PutValue(dlcLeft, _variant_t(""));
		pRow->PutValue(dlcRight, _variant_t(""));

		m_pList->AddRowAtEnd(pRow, NULL);	
	}

	//now loop through the left list modifying the rows
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstRow();
	for (int i = 0; i < m_aryLeftList.GetSize(); i++) {

		CPatSummaryList *pItem = m_aryLeftList.GetAt(i);

		for (int j = 0; j < pItem->m_paryObjects->GetSize(); j++) {

			CPatSummaryListInfoObject *infObj = pItem->m_paryObjects->GetAt(j);
			if (infObj && pRow) {
				pRow->PutValue(dlcLeft, _variant_t(infObj->m_strLabel));								
				if (!infObj->m_strTabLink.IsEmpty()) {
					pRow->PutCellForeColor(dlcLeft, infObj->m_cColor);
					pRow->PutCellLinkStyle(dlcLeft, NXDATALIST2Lib::dlLinkStyleTrue);
					pRow->PutValue(dlcLeftLink, _variant_t(infObj->m_strTabLink));
				}
			}
			pRow = pRow->GetNextRow();
		}
	}

	pRow = m_pList->GetFirstRow();
	for (int i = 0; i < m_aryRightList.GetSize(); i++) {

		CPatSummaryList *pItem = m_aryRightList.GetAt(i);

		for (int j = 0; j < pItem->m_paryObjects->GetSize(); j++) {

			CPatSummaryListInfoObject *infObj = pItem->m_paryObjects->GetAt(j);
			if (infObj && pRow) {
				pRow->PutValue(dlcRight, _variant_t(infObj->m_strLabel));				
				if (!infObj->m_strTabLink.IsEmpty()) {
					pRow->PutCellForeColor(dlcRight, infObj->m_cColor);
					pRow->PutCellLinkStyle(dlcRight, NXDATALIST2Lib::dlLinkStyleTrue);
					pRow->PutValue(dlcRightLink, _variant_t(infObj->m_strTabLink));
				}
			}
			pRow = pRow->GetNextRow();
		}
	}
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::LoadTopList()
{
	try {

		CWaitCursor pWait;

		m_nxstaticStatusLabel.SetWindowText("Loading...");

		//clear the list
		m_pList->Clear();

		// (j.gruber 2010-06-10 10:52) - PLID 26363 - Make one round trip to server
		CString strSql = GetTotalSql();		

		_RecordsetPtr rsAllRecs = CreateRecordsetStd(strSql);

		if (rsAllRecs) {					

			DisplayPatientInformation(rsAllRecs);

			//next is emerg Contact Info
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayEmergencyContactInfo(rsAllRecs);

			//next is Custom 1
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 1, "PatSumCustom1");

			//next is Custom 2
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 2, "PatSumCustom2");

			//next is Custom 3
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 3, "PatSumCustom3");

			//next is Custom 4
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 4, "PatSumCustom4");

			//next is Patient Notes
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralField(rsAllRecs, "Patient Notes", "tabG1", "PatSumPatNotes");

			//next is Provider			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralField(rsAllRecs, "Main Physician", "tabG1", "PatSumMainPhysician");

			//next is Pat Coord			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralField(rsAllRecs, "Patient Coordinator", "tabG1", "PatSumPatCoord");

			//next is First Contact Date			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralDate(rsAllRecs, "First Contact Date", "tabG1", "PatSumFirstContactDate");

			//next is Entered By			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralField(rsAllRecs, "Entered By", "tabG1", "PatSumEnteredBy");

			// (j.jones 2012-02-01 17:16) - PLID 46145 - added global periods
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralField(rsAllRecs, "Global Periods", "tabBill", "PatSumGlobalPeriods");

			//next is Emplyment Info			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayEmploymentInfo(rsAllRecs);

			//next is Race			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralField(rsAllRecs, "Race", "tabG2", "PatSumRace");

			//next is Ethnicity			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralField(rsAllRecs, "Ethnicity", "tabG2", "PatSumEthnicity");

			//next is Language			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayLanguageField(rsAllRecs);

			//next is ICd9's			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralField(rsAllRecs, "Default Diagnosis Codes", "tabG2", "PatSumDiagCodes");

			//next is Current Illness Date			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralDate(rsAllRecs, "Current Illness Date", "tabG2", "PatSumCurIllDate");

			//next is Patient Type			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralField(rsAllRecs, "Patient Type", "tabG2", "PatSumPatType");

			//next is Location			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralField(rsAllRecs, "Location", "tabG2", "PatSumLocation");

			//next is Referring Information			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayReferralInformation(rsAllRecs);

			//next is Warning Information			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayWarningInformation(rsAllRecs);

			//skipping Serialized Products and NexWeb Login Info

			//next is Last Modified Date			
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralDate(rsAllRecs, "Last Modified Date", "tabG2", "PatSumLastModified", TRUE);

			//next is Custom Text1
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 11, "PatSumCustomText1");

			//next is Custom Text 2
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 12, "PatSumCustomText2");

			//next is Custom Text 3
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 13, "PatSumCustomText3");

			//next is Custom Text 4
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 14, "PatSumCustomText4");

			//next is Custom Text 5
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 15, "PatSumCustomText5");

			//next is Custom Text 6
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 16, "PatSumCustomText6");

			//next is Custom Text 7
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 90, "PatSumCustomText7");

			//next is Custom Text 8
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 91, "PatSumCustomText8");

			//next is Custom Text 9
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 92, "PatSumCustomText9");

			//next is Custom Text 10
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 93, "PatSumCustomText10");

			//next is Custom Text 11
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 94, "PatSumCustomText11");

			//next is Custom Text 12
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 95, "PatSumCustomText12");

			//next is Custom Note
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 17, "PatSumCustomNote");

			//next is Custom List 1
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 21, "PatSumCustomList1");

			//next is Custom List 2
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 22, "PatSumCustomList2");

			//next is Custom List 3
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 23, "PatSumCustomList3");

			//next is Custom List 4
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 24, "PatSumCustomList4");

			//next is Custom List 5
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 25, "PatSumCustomList5");

			//next is Custom List 6
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 26, "PatSumCustomList6");

			//next is Custom Contact
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 31, "PatSumCustomContact");

			//next is Custom Check 1
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 41, "PatSumCustomCheck1");

			//next is Custom Check 2
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 42, "PatSumCustomCheck2");

			//next is Custom Check 3
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 43, "PatSumCustomCheck3");

			//next is Custom Check 4
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 44, "PatSumCustomCheck4");

			//next is Custom Check 5
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 45, "PatSumCustomCheck5");

			//next is Custom Check 6
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 46, "PatSumCustomCheck6");

			//next is Custom Date 1
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 51, "PatSumCustomDate1");

			//next is Custom Date 2
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 52, "PatSumCustomDate2");

			//next is Custom Date 3
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 53, "PatSumCustomDate3");

			//next is Custom Date 4
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCustomField(rsAllRecs, 54, "PatSumCustomDate4");

			//next is InsuredPartyInfo
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayInsPartyInfo(rsAllRecs);

			//next is Current Meds
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayCurrentMeds(rsAllRecs);

			//next is Allergies
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayAllergies(rsAllRecs);

			//next is Last Appt Info
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayLastAppt(rsAllRecs);

			//next is Last Diag Codes Billed
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayLastDiagCodesBilled(rsAllRecs);

			//next is Patient Balance
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralCurrency(rsAllRecs, "Patient Balance", "tabBill", "PatSumPatBal");

			//next is Ins Balance
			rsAllRecs = rsAllRecs->NextRecordset(NULL);
			DisplayGeneralCurrency(rsAllRecs, "Insurance Balance", "tabBill", "PatSumInsBal");		

			//now each list is filled, so we need to sort it
			SortArray(&m_aryLeftList);
			SortArray(&m_aryRightList);

			AddArraysToList();			

			//now we can get rid of the arrays
			DeleteArrays();
		}

		m_nxstaticStatusLabel.SetWindowText("");

	}NxCatchAll(__FUNCTION__);
}




void CPatientSummaryDlg::LoadBottomList()
{
	try {

		CWaitCursor pWait;

		m_nxstaticStatusLabel.SetWindowText("Loading...");
		
		//load each object into the list
		DisplayAppointments();		
		DisplayBills();		
		DisplayQuotes();		
		DisplayPayments();
		DisplayEMNs();
		DisplayPrescriptions();		
		DisplayFollowUps();		
		DisplayNotes();	
		DisplayHistory();
		DisplayTracking();		
		DisplayLabs();		

		m_nxstaticStatusLabel.SetWindowText("");

	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::DeleteArrays()
{

	//left list
	for(int i=m_aryLeftList.GetSize()-1;i>=0;i--) {
		CPatSummaryList *pList = m_aryLeftList.GetAt(i);
		m_aryLeftList.RemoveAt(i);
		
		if (pList) {
			CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryInfo  = pList->m_paryObjects;
			for(int j=paryInfo->GetSize()-1;j>=0;j--) {
				CPatSummaryListInfoObject *pItem = paryInfo->GetAt(j);
				paryInfo->RemoveAt(j);
				
				if (pItem) {					
					delete pItem;
				}
			}			
			delete paryInfo;
			delete pList;
		}
	}
	m_aryLeftList.RemoveAll();

	//right list
	for(int i=m_aryRightList.GetSize()-1;i>=0;i--) {
		CPatSummaryList *pList = m_aryRightList.GetAt(i);
		m_aryRightList.RemoveAt(i);
		if (pList) {
			CArray<CPatSummaryListInfoObject*, CPatSummaryListInfoObject*> *paryInfo  = pList->m_paryObjects;
			for(int j=pList->m_paryObjects->GetSize()-1;j>=0;j--) {
				CPatSummaryListInfoObject *pItem = paryInfo->GetAt(j);
				paryInfo->RemoveAt(j);

				if (pItem) {
					delete pItem;
				}
			}
			delete paryInfo;
			delete pList;
		}
	}
	m_aryRightList.RemoveAll();

}

void CPatientSummaryDlg::DisplayAppointments()
{
	try {

		CWaitCursor pWait;

		//check licensing
		//TES 12/10/2008 - PLID 32145 - New function for checking scheduler licensing.
		if(!g_pLicense || !g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent)) {
			return;
		}

		//make sure they have unrestricted view permission
		if(!(GetCurrentUserPermissions(bioAppointment) & sptView)) {
			return;
		}

		m_nxstaticStatusLabel.SetWindowText("Loading Appointments...");

		PeekAndPump();

		CWaitCursor pWait2;

		//find all un-cancelled, non no-show appts. for this patient where the appt. is in the future,
		//or within the past three months, but only show up to three past appts.
		// (j.jones 2010-08-11 17:02) - PLID 38508 - this logic intentionally loads the last appt. prior to today's date
		_RecordsetPtr rs = CreateParamRecordset("SELECT ResBasicQ.Date, ResBasicQ.StartTime, "
			"dbo.GetResourceString(ResBasicQ.ID) AS Resource, "
			"CASE WHEN ResBasicQ.AptPurposeName <> '' THEN AptTypeT.Name + ' - ' + ResBasicQ.AptPurposeName ELSE AptTypeT.Name END AS Purpose, "
			"CASE WHEN AptTypeT.ID IS NULL THEN 0 ELSE AptTypeT.Color END AS Color, "
			"Convert(bit, CASE WHEN dbo.AsDateNoTime(Date) < dbo.AsDateNoTime(GetDate()) THEN 1 ELSE 0 END) AS IsPastAppt "
			"FROM (SELECT AppointmentsT.ID, AppointmentsT.PatientID, AppointmentsT.AptTypeID, dbo.GetPurposeString(AppointmentsT.ID) AS AptPurposeName, CONVERT(datetime, CONVERT(varchar, AppointmentsT.StartTime, 23)) AS Date, convert(datetime, RIGHT(CONVERT(varchar, AppointmentsT.StartTime), 7)) AS StartTime, AppointmentsT.Confirmed, AppointmentsT.Notes, AppointmentsT.LocationID, AppointmentsT.RecordID, AppointmentsT.Status, PersonT.Archived, AppointmentsT.ShowState, AppointmentsT.CreatedDate, AppointmentsT.LastLM, AppointmentsT.CreatedLogin, AppointmentsT.ModifiedDate, AppointmentsT.ModifiedLogin "
			"FROM AppointmentsT "
			"LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
			"WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3) ResBasicQ "
			"INNER JOIN AptTypeT ON ResBasicQ.AptTypeID = AptTypeT.ID "
			"WHERE PatientID = {INT} AND dbo.AsDateNoTime(ResBasicQ.Date) >= dbo.AsDateNoTime(DATEADD(month, -3, GetDate())) "
			"ORDER BY Date DESC, StartTime ASC", m_nPatientID);

		long nCountPastAppts = 0;

		while(!rs->eof) {
			
			COleDateTime dtDate = AdoFldDateTime(rs, "Date");
			COleDateTime dtTime = AdoFldDateTime(rs, "StartTime");
			CString strResource = AdoFldString(rs, "Resource", "");
			CString strPurpose = AdoFldString(rs, "Purpose", "");
			long nColor = AdoFldLong(rs, "Color", 0);

			OLE_COLOR color = nColor;
			// (z.manning 2015-04-23 17:19) - NX-100448 - Made a function for this
			nColor = GetDarkerColorForApptText(nColor);

			BOOL bIsPastAppt = AdoFldBool(rs, "IsPastAppt", FALSE);

			if(bIsPastAppt) {
				nCountPastAppts++;

				if(nCountPastAppts > 3) {
					//don't display more than 3 old appts., leave if we have already done so
					PeekAndPump();
					return;
				}
			}

			if(strPurpose.IsEmpty()) {
				strPurpose = "<No Type, No Purpose>";
			}

			CString strDescription;
			strDescription.Format("%s - %s, Resource: %s", FormatDateTimeForInterface(dtTime, DTF_STRIP_SECONDS, dtoTime), strPurpose, strResource);

			//add to our list
			IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
			pRow->PutValue(slcDate, _variant_t(dtDate, VT_DATE));
			pRow->PutValue(slcCategory, "Appointment");
			pRow->PutValue(slcDescription, _bstr_t(strDescription));
			m_SummaryList->AddRowSorted(pRow, NULL);

			pRow->PutCellForeColor(slcDescription, color);

			rs->MoveNext();
		}
		rs->Close();

		PeekAndPump();

	}NxCatchAll("Error in CPatientSummaryDlg::DisplayAppointments");
}

void CPatientSummaryDlg::DisplayBills()
{
	try {

		CWaitCursor pWait1;

		//check licensing
		if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
			return;
		}

		//make sure they have unrestricted read permission
		if(!(GetCurrentUserPermissions(bioPatientBilling) & sptRead)) {
			return;
		}

		m_nxstaticStatusLabel.SetWindowText("Loading Bills...");

		PeekAndPump();

		CWaitCursor pWait2;

		//find all bills that are unpaid, plus the last three bills that are fully paid off
		_RecordsetPtr rs = CreateParamRecordset("SELECT BillsT.Date AS Date, BillsT.Description, "
			"dbo.GetBillTotal(BillsT.ID) AS BillTotal, Coalesce(AppliesQ.ApplyTotal, Convert(money, 0)) AS ApplyTotal, "
			"Convert(bit, CASE WHEN Coalesce(AppliesQ.ApplyTotal, Convert(money, 0)) < dbo.GetBillTotal(BillsT.ID) THEN 0 ELSE 1 END) AS IsPaidOff "
			"FROM BillsT "
			"INNER JOIN (SELECT ChargesT.BillID, Sum(AppliesT.Amount) AS ApplyTotal "
			"	FROM ChargesT "
			"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
			"	LEFT JOIN AppliesT ON ChargesT.ID = AppliesT.DestID "
			"	WHERE LineItemT.Deleted = 0 "
			"	GROUP BY ChargesT.BillID) AS AppliesQ ON BillsT.ID = AppliesQ.BillID "
			"WHERE BillsT.Deleted = 0 AND EntryType = 1 "
			"AND BillsT.PatientID = {INT} "
			"ORDER BY Convert(bit, CASE WHEN Coalesce(AppliesQ.ApplyTotal, Convert(money, 0)) < dbo.GetBillTotal(BillsT.ID) THEN 0 ELSE 1 END) ASC, BillsT.Date DESC", m_nPatientID);

		long nCountPaidBills = 0;

		while(!rs->eof) {
			
			COleDateTime dtDate = AdoFldDateTime(rs, "Date");
			CString strBillDescription = AdoFldString(rs, "Description", "");
			COleCurrency cyBillTotal = AdoFldCurrency(rs, "BillTotal", COleCurrency(0,0));
			COleCurrency cyApplyTotal = AdoFldCurrency(rs, "ApplyTotal", COleCurrency(0,0));
			BOOL bIsPaidOff = AdoFldBool(rs, "IsPaidOff", FALSE);

			if(bIsPaidOff) {
				nCountPaidBills++;

				if(nCountPaidBills > 3) {
					//don't display more than 3 paid bills, which are sorted last, leave if we have already done so
					PeekAndPump();
					return;
				}
			}

			if(strBillDescription.IsEmpty()) {
				strBillDescription = "<No Description>";
			}

			CString strDescription;
			strDescription.Format("%s - Total: %s, Paid: %s, Balance: %s", strBillDescription, FormatCurrencyForInterface(cyBillTotal, TRUE, TRUE, TRUE), FormatCurrencyForInterface(cyApplyTotal, TRUE, TRUE, TRUE), FormatCurrencyForInterface(cyBillTotal - cyApplyTotal, TRUE, TRUE, TRUE));

			//add to our list
			IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
			pRow->PutValue(slcDate, _variant_t(dtDate, VT_DATE));
			pRow->PutValue(slcCategory, "Bill");
			pRow->PutValue(slcDescription, _bstr_t(strDescription));
			m_SummaryList->AddRowSorted(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();

		PeekAndPump();

	}NxCatchAll("Error in CPatientSummaryDlg::DisplayBills");
}

void CPatientSummaryDlg::DisplayQuotes()
{
	try {

		CWaitCursor pWait;

		//check licensing
		if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcQuotes, CLicense::cflrSilent)) {
			return;
		}

		//make sure they have unrestricted read permission
		if(!(GetCurrentUserPermissions(bioPatientQuotes) & sptRead)) {
			return;
		}

		m_nxstaticStatusLabel.SetWindowText("Loading Quotes...");

		PeekAndPump();

		CWaitCursor pWait2;
		
		//first find all active, unbilled quotes
		// (j.gruber 2009-03-18 12:36) - PLID 33574 - update discount structure
		// (j.jones 2011-06-17 14:09) - PLID 38347 - fixed quote total calculation to account for modifiers
		_RecordsetPtr rs = CreateParamRecordset("SELECT [PatientBillsQ].ID, [PatientBillsQ].Description, [PatientBillsQ].Date, "
			"Sum(Round(Convert(money,((([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))) + "
			"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate-1)) + "
			"(([Amount]*[Quantity]*(CASE WHEN(CPTMultiplier1 Is Null) THEN 1 ELSE CPTMultiplier1 END)*(CASE WHEN CPTMultiplier2 Is Null THEN 1 ELSE CPTMultiplier2 END)*(CASE WHEN(CPTMultiplier3 Is Null) THEN 1 ELSE CPTMultiplier3 END)*(CASE WHEN CPTMultiplier4 Is Null THEN 1 ELSE CPTMultiplier4 END)*(CASE WHEN([TotalPercentOff] Is Null) THEN 1 ELSE ((100-Convert(float,[TotalPercentOff]))/100) END)-(CASE WHEN([TotalDiscount] Is Null OR (Amount = 0 AND OthrBillFee > 0)) THEN 0 ELSE [TotalDiscount] END))*(TaxRate2-1)) "
			")),2)) AS Total "
			"FROM ((SELECT BillsT.*, (SELECT Max(BillID) FROM BilledQuotesT "
			"	WHERE BilledQuotesT.QuoteID = BillsT.ID "
			"	AND BilledQuotesT.BillID IN (SELECT ID FROM BillsT WHERE Deleted = 0 AND EntryType = 1)) AS HasBeenBilled "
			"FROM BillsT WHERE (((BillsT.PatientID) = {INT}) AND ((BillsT.Deleted)=0)) AND BillsT.Active = 1) AS PatientBillsQ "
			"LEFT JOIN (SELECT LineItemT.*, ChargesT.BillID, ChargesT.DoctorsProviders "
			"	FROM LineItemT "
			"	INNER JOIN ChargesT ON LineItemT.ID = ChargesT.ID "
			"	WHERE (((LineItemT.PatientID) = {INT}) "
			"	AND ((LineItemT.Deleted)=0) AND ((LineItemT.Type)>=10))"
			") AS PatientChargesQ ON [PatientBillsQ].ID = [PatientChargesQ].BillID) "
			"LEFT JOIN ChargesT ON [PatientChargesQ].ID = ChargesT.ID "
			"LEFT JOIN (SELECT ChargeID, SUM(Percentoff) as TotalPercentOff FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalPercentageQ ON ChargesT.ID = TotalPercentageQ.ChargeID  "
			"LEFT JOIN (SELECT ChargeID, SUM(Discount) as TotalDiscount FROM ChargeDiscountsT WHERE DELETED = 0 GROUP BY ChargeID) TotalDiscountQ ON ChargesT.ID = TotalDiscountQ.ChargeID  "
			"WHERE [PatientBillsQ].EntryType=2 AND (HasBeenBilled = 0 OR HasBeenBilled Is Null) "
			"AND PatientBillsQ.ID NOT IN (SELECT QuoteID FROM PackagesT) "
			"GROUP BY [PatientBillsQ].ID, [PatientBillsQ].Date, [PatientBillsQ].Description, PatientBillsQ.HasBeenBilled "
			"ORDER BY [PatientBillsQ].Date DESC", m_nPatientID, m_nPatientID);

		while(!rs->eof) {
			
			long nQuoteID = AdoFldLong(rs, "ID");
			COleDateTime dtDate = AdoFldDateTime(rs, "Date");
			CString strQuoteDescription = AdoFldString(rs, "Description", "");
			COleCurrency cyQuoteTotal = AdoFldCurrency(rs, "Total", COleCurrency(0,0));

			if(strQuoteDescription.IsEmpty()) {
				strQuoteDescription = "<No Description>";
			}

			COleCurrency cyPrePays = CalculatePrePayments(m_nPatientID, nQuoteID, GetRemotePropertyInt("IncludeAllPrePaysInPopUps", 1, 0, "<None>", TRUE) == 1);

			COleCurrency cyBalance = cyQuoteTotal - cyPrePays;
			if(cyBalance < COleCurrency(0,0)) {
				cyBalance = COleCurrency(0,0);
			}

			CString strDescription;
			strDescription.Format("%s - Total: %s, PrePaid: %s, Balance: %s", strQuoteDescription, FormatCurrencyForInterface(cyQuoteTotal, TRUE, TRUE, TRUE), FormatCurrencyForInterface(cyPrePays, TRUE, TRUE, TRUE), FormatCurrencyForInterface(cyBalance, TRUE, TRUE, TRUE));

			//add to our list
			IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
			pRow->PutValue(slcDate, _variant_t(dtDate, VT_DATE));
			pRow->PutValue(slcCategory, "Quote");
			pRow->PutValue(slcDescription, _bstr_t(strDescription));
			m_SummaryList->AddRowSorted(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();

		m_nxstaticStatusLabel.SetWindowText("Loading Packages...");

		PeekAndPump();

		CWaitCursor pWait3;

		//now find all active, incompleted packages
		rs = CreateParamRecordset("SELECT PackagesQ.*, BillsT.Description, BillsT.Date FROM "
			"	(SELECT PackagesT.QuoteID, TotalAmount, CurrentAmount, OriginalCurrentAmount, Type, "
			"	(CASE WHEN Type = 1 THEN PackagesT.TotalCount WHEN Type = 2 THEN PackageChargesQ.MultiUseTotalCount ELSE 0 END) AS TotalCount, "
			"	(CASE WHEN Type = 1 THEN PackagesT.CurrentCount WHEN Type = 2 THEN PackageChargesQ.MultiUseCurrentCount ELSE 0 END) AS CurrentCount "
			"	FROM PackagesT "
			"	LEFT JOIN "
			"		(SELECT ChargesT.BillID, Sum(Quantity) AS MultiUseTotalCount, Sum(PackageQtyRemaining) AS MultiUseCurrentCount "
			"		FROM ChargesT INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID WHERE Deleted = 0 "
			"		AND (LineItemT.Amount > Convert(money,0) OR ChargesT.OthrBillFee = Convert(money,0)) "
			"		GROUP BY BillID "
			"		) AS PackageChargesQ ON PackagesT.QuoteID = PackageChargesQ.BillID "
			"	) AS PackagesQ "
			"INNER JOIN BillsT ON PackagesQ.QuoteID = BillsT.ID "
			"WHERE BillsT.Deleted = 0 AND BillsT.PatientID = {INT} AND BillsT.Active = 1 "
			"AND CurrentCount > 0 "
			"ORDER BY BillsT.Date DESC", m_nPatientID);

		while(!rs->eof) {

			COleCurrency cyTotalCostNoTax = COleCurrency(0,0), cyTotalCostWithTax = COleCurrency(0,0);
			COleCurrency cyRemBalanceNoTax = COleCurrency(0,0), cyRemBalanceWithTax = COleCurrency(0,0);
			COleCurrency cyUnBilledNoTax = COleCurrency(0,0), cyUnBilledWithTax = COleCurrency(0,0);
			
			long nPackageID = AdoFldLong(rs, "QuoteID");
			COleDateTime dtDate = AdoFldDateTime(rs, "Date");
			CString strQuoteDescription = AdoFldString(rs, "Description", "");

			if(strQuoteDescription.IsEmpty()) {
				strQuoteDescription = "<No Description>";
			}
			
			long nType = AdoFldLong(rs, "Type");
			CString strType = "Repeatable";
			if(nType == 2) {
				strType = "Multi-Use";
			}

			cyTotalCostNoTax = AdoFldCurrency(rs, "TotalAmount", COleCurrency(0,0));
			cyUnBilledNoTax = AdoFldCurrency(rs, "CurrentAmount", COleCurrency(0,0));

			double dblTotalCount = 0.0, dblRemCount = 0.0;

			_variant_t var = rs->Fields->Item["TotalCount"]->Value;
			if(var.vt == VT_I4) {
				dblTotalCount = (double)VarLong(var);
			}
			else if(var.vt == VT_R8) {
				dblTotalCount = VarDouble(var);
			}
			else {
				dblTotalCount = 0.0;
			}

			var = rs->Fields->Item["CurrentCount"]->Value;
			if(var.vt == VT_I4) {
				dblRemCount = (double)VarLong(var);
			}
			else if(var.vt == VT_R8) {
				dblRemCount = VarDouble(var);
			}
			else {
				dblRemCount = 0.0;
			}

			COleCurrency cyPrePays = CalculatePrePayments(m_nPatientID, nPackageID, GetRemotePropertyInt("IncludeAllPrePaysInPopUps", 1, 0, "<None>", TRUE) == 1);

			// (j.jones 2008-08-12 10:54) - PLID 24624 - this now requires the patient ID
			CalculateRemainingPackageBalance(nPackageID, m_nPatientID, rs->Fields->Item["OriginalCurrentAmount"]->Value, cyRemBalanceNoTax, cyRemBalanceWithTax);
			
			BOOL bHasTax = FALSE;

			cyTotalCostWithTax = CalculateTotalPackageValueWithTax(nPackageID);

			//if the balances are different, tax exists, so make sure we also grab the other amounts with tax
			if(cyTotalCostNoTax != cyTotalCostWithTax) {
				bHasTax = TRUE;

				cyUnBilledWithTax = CalculateRemainingPackageValueWithTax(nPackageID);
			}
			else {
				cyRemBalanceWithTax = cyRemBalanceNoTax;
				cyUnBilledWithTax = cyUnBilledNoTax;
			}

			CString strDescription;
			strDescription.Format("%s, Type: %s", strQuoteDescription, strType);

			strDescription += ", Package Cost: ";
			strDescription += FormatCurrencyForInterface(cyTotalCostNoTax, TRUE, TRUE, TRUE);

			if(bHasTax) {
				strDescription += ", Cost With Tax: ";
				strDescription += FormatCurrencyForInterface(cyTotalCostWithTax, TRUE, TRUE, TRUE);
			}

			strDescription += ", Unbilled Amount: ";
			strDescription += FormatCurrencyForInterface(cyUnBilledNoTax, TRUE, TRUE, TRUE);

			if(bHasTax) {
				strDescription += ", With Tax: ";
				strDescription += FormatCurrencyForInterface(cyUnBilledWithTax, TRUE, TRUE, TRUE);
			}

			strDescription += ", Remaining Balance: ";
			strDescription += FormatCurrencyForInterface(cyRemBalanceNoTax, TRUE, TRUE, TRUE);

			if(bHasTax) {
				strDescription += ", With Tax: ";
				strDescription += FormatCurrencyForInterface(cyRemBalanceWithTax, TRUE, TRUE, TRUE);
			}

			//add to our list
			IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
			pRow->PutValue(slcDate, _variant_t(dtDate, VT_DATE));
			pRow->PutValue(slcCategory, "Package");
			pRow->PutValue(slcDescription, _bstr_t(strDescription));
			m_SummaryList->AddRowSorted(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();

		PeekAndPump();

	}NxCatchAll("Error in CPatientSummaryDlg::DisplayQuotes");
}

void CPatientSummaryDlg::DisplayPayments()
{
	try {

		CWaitCursor pWait;

		//check licensing
		if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
			return;
		}

		//make sure they have unrestricted read permission
		if(!(GetCurrentUserPermissions(bioPatientBilling) & sptRead)) {
			return;
		}

		m_nxstaticStatusLabel.SetWindowText("Loading Payments...");

		PeekAndPump();

		CWaitCursor pWait2;

		//find all payments that are not fully applied, plus the last three payments that were fully applied,
		//ignoring amounts applied to the payments
		_RecordsetPtr rs = CreateParamRecordset("SELECT LineItemT.Date AS Date, LineItemT.Description, "
			"LineItemT.Amount, Coalesce(AppliesQ.ApplyTotal, Convert(money, 0)) AS ApplyTotal, "
			"Convert(bit, CASE WHEN Coalesce(AppliesQ.ApplyTotal, Convert(money, 0)) < LineItemT.Amount THEN 0 ELSE 1 END) AS IsFullyApplied "
			"FROM LineItemT "
			"INNER JOIN PaymentsT ON LineItemT.ID = PaymentsT.ID "
			"LEFT JOIN (SELECT AppliesT.SourceID, Sum(AppliesT.Amount) AS ApplyTotal "
			"	FROM AppliesT "
			"	INNER JOIN PaymentsT ON AppliesT.SourceID = PaymentsT.ID "
			"	GROUP BY AppliesT.SourceID) AS AppliesQ ON LineItemT.ID = AppliesQ.SourceID "
			"WHERE LineItemT.Deleted = 0 AND LineItemT.Type = 1 "
			"AND LineItemT.PatientID = {INT} "
			"ORDER BY Convert(bit, CASE WHEN Coalesce(AppliesQ.ApplyTotal, Convert(money, 0)) < LineItemT.Amount THEN 0 ELSE 1 END) ASC, LineItemT.Date DESC", m_nPatientID);

		long nCountFullyApplied = 0;

		while(!rs->eof) {
			
			COleDateTime dtDate = AdoFldDateTime(rs, "Date");
			CString strPayDescription = AdoFldString(rs, "Description", "");
			COleCurrency cyPaymentAmount = AdoFldCurrency(rs, "Amount", COleCurrency(0,0));
			COleCurrency cyApplyTotal = AdoFldCurrency(rs, "ApplyTotal", COleCurrency(0,0));
			BOOL bIsFullyApplied = AdoFldBool(rs, "IsFullyApplied", FALSE);

			if(bIsFullyApplied) {
				nCountFullyApplied++;

				if(nCountFullyApplied > 3) {
					//don't display more than 3 fully-applied payments, which are sorted last, leave if we have already done so
					PeekAndPump();
					return;
				}
			}

			if(strPayDescription.IsEmpty()) {
				strPayDescription = "<No Description>";
			}

			CString strDescription;
			strDescription.Format("%s - Total: %s, Applied: %s, Amt. Unapplied: %s", strPayDescription, FormatCurrencyForInterface(cyPaymentAmount, TRUE, TRUE, TRUE), FormatCurrencyForInterface(cyApplyTotal, TRUE, TRUE, TRUE), FormatCurrencyForInterface(cyPaymentAmount - cyApplyTotal, TRUE, TRUE, TRUE));

			//add to our list
			IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
			pRow->PutValue(slcDate, _variant_t(dtDate, VT_DATE));
			pRow->PutValue(slcCategory, "Payment");
			pRow->PutValue(slcDescription, _bstr_t(strDescription));
			m_SummaryList->AddRowSorted(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();

		PeekAndPump();

	}NxCatchAll("Error in CPatientSummaryDlg::DisplayPayments");
}

void CPatientSummaryDlg::DisplayEMNs()
{
	try {

		CWaitCursor pWait;

		//check licensing for either L1 or L2
		if(!g_pLicense || g_pLicense->HasEMR(CLicense::cflrSilent) == 0) {
			return;
		}

		//make sure they have unrestricted read permission
		if(!(GetCurrentUserPermissions(bioPatientEMR) & sptRead)) {
			return;
		}

		m_nxstaticStatusLabel.SetWindowText("Loading EMNs...");

		PeekAndPump();

		CWaitCursor pWait2;

		//find the last five EMNs, by date, and any in the future (which would be odd, but let's just do it anyways)
		// (z.manning 2011-05-20 11:56) - PLID 33114 - Factor in EMR charting permissions here
		// (j.jones 2011-07-05 17:49) - PLID 44432 - supported custom statuses
		_RecordsetPtr rs = CreateParamRecordset("SELECT EMRMasterT.Date, EMRMasterT.Description, "
			"dbo.GetEmrString(EMRMasterT.ID) AS Procedures, "
			"EMRStatusListT.Name AS Status, "
			"Convert(bit, CASE WHEN EMRMasterT.Date < GetDate() THEN 1 ELSE 0 END) AS IsPastEMN "
			"FROM EMRMasterT "
			"LEFT JOIN EmnTabChartsLinkT ON EmrMasterT.ID = EmnTabChartsLinkT.EmnID "
			"LEFT JOIN EMRStatusListT ON EMRMasterT.Status = EMRStatusListT.ID "
			"WHERE EMRMasterT.Deleted = 0 AND EMRMasterT.PatientID = {INT} {SQL} "
			"ORDER BY Convert(bit, CASE WHEN EMRMasterT.Date < GetDate() THEN 1 ELSE 0 END) ASC, EMRMasterT.Date DESC "
			, m_nPatientID, GetEmrChartPermissionFilter());

		long nCountPastEMNs = 0;

		while(!rs->eof) {
			
			COleDateTime dtDate = AdoFldDateTime(rs, "Date");
			CString strEMNDescription = AdoFldString(rs, "Description", "");
			CString strProcedures = AdoFldString(rs, "Procedures", "");
			CString strStatus = AdoFldString(rs, "Status", "");
			BOOL bIsPastEMN = AdoFldBool(rs, "IsPastEMN", FALSE);

			if(bIsPastEMN) {
				nCountPastEMNs++;

				if(nCountPastEMNs > 5) {
					//don't display more than 5 EMNs in the past, leave if we have already done so
					PeekAndPump();
					return;
				}
			}

			if(strEMNDescription.IsEmpty()) {
				strEMNDescription = "<No Description>";
			}
			if(strProcedures.IsEmpty()) {
				strProcedures = "<No Procedures>";
			}

			CString strDescription;
			strDescription.Format("%s, Procedures: %s, Status: %s", strEMNDescription, strProcedures, strStatus);

			//add to our list
			IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
			pRow->PutValue(slcDate, _variant_t(dtDate, VT_DATE));
			pRow->PutValue(slcCategory, "EMN");
			pRow->PutValue(slcDescription, _bstr_t(strDescription));
			m_SummaryList->AddRowSorted(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();

		PeekAndPump();

	}NxCatchAll("Error in CPatientSummaryDlg::DisplayEMNs");
}

void CPatientSummaryDlg::DisplayPrescriptions()
{
	try {

		CWaitCursor pWait;

		//check licensing
		if(!g_pLicense->CheckForLicense(CLicense::lcPatient, CLicense::cflrJustCheckingLicenseNoUI)) {
			return;
		}
		
		//make sure they have unrestricted read permission
		if(!(GetCurrentUserPermissions(bioPatientMedication) & sptRead)) {
			return;
		}

		m_nxstaticStatusLabel.SetWindowText("Loading Prescriptions...");

		PeekAndPump();

		CWaitCursor pWait2;

		//Find the last three dates where prescriptions were written, as in if 5 prescriptions were
		//written on one day, and 3 were prescribed on one day the previous visit, we'd want to show all 8,
		//plus whatever was prescribed on the visit prior to that.
		//It should be impossible - and illegal - to prescribe something in the future, so do not
		//bother attempting to handle that differently. Consider it being one of the three last visits.
		//TES 2/10/2009 - PLID 33002 - Changed Description to PatientExplanation, PillsPerBottle to Quantity
		_RecordsetPtr rs = CreateParamRecordset("SELECT PrescriptionDate, EMRDataT.Data AS MedicationName, "
			"PatientMedications.PatientExplanation, PatientMedications.RefillsAllowed, PatientMedications.Quantity,  "
			"PatientMedications.Unit, LocationsT.Name AS Pharmacy, "
			"PersonT.Last + ', ' + PersonT.First + ' ' + PersonT.Middle AS ProviderName "
			"FROM PatientMedications "
			"INNER JOIN DrugList ON PatientMedications.MedicationID = DrugList.ID "
			"INNER JOIN EMRDataT ON DrugList.EMRDataID = EMRDataT.ID "
			"LEFT JOIN PersonT ON PatientMedications.ProviderID = PersonT.ID "
			"LEFT JOIN LocationsT ON PatientMedications.PharmacyID = LocationsT.ID "
			"WHERE PatientMedications.Deleted = 0 AND PatientID = {INT} "
			"ORDER BY PrescriptionDate DESC", m_nPatientID);

		long nCountPastDates = 0;
		COleDateTime dtLastDate;

		while(!rs->eof) {
			
			//for some inexplicable reason, this can be null
			_variant_t varDate = rs->Fields->Item["PrescriptionDate"]->Value;

			//we only try to show the last three visits, but don't track
			//if it is NULL - that is bad data, if we encounter if before finding
			//three dates, we'll just show it anyways
			if(varDate.vt == VT_DATE) {
				COleDateTime dtDate = VarDateTime(varDate);

				if(nCountPastDates == 0 || dtLastDate != dtDate) {
					dtLastDate = dtDate;
					nCountPastDates++;

					if(nCountPastDates > 3) {
						//don't display more than 3 visits' worth of prescriptions
						PeekAndPump();
						return;
					}
				}
			}

			CString strMedicationName = AdoFldString(rs, "MedicationName", "");
			CString strExplanation = AdoFldString(rs, "PatientExplanation", "");
			CString strRefills = AdoFldString(rs, "RefillsAllowed", "");
			CString strQuantity = AdoFldString(rs, "Quantity", "");
			CString strUnit = AdoFldString(rs, "Unit", "");
			CString strProvider = AdoFldString(rs, "ProviderName", "");
			CString strPharmacy = AdoFldString(rs, "Pharmacy", "");

			if(strExplanation.IsEmpty()) {
				strExplanation = "<No Description>";
			}
			if(strRefills.IsEmpty()) {
				strRefills = "<None>";
			}
			if(strQuantity.IsEmpty()) {
				strQuantity = "<None>";
			}
			if(strUnit.IsEmpty()) {
				strUnit = "<None>";
			}
			if(strProvider.IsEmpty()) {
				strProvider = "<Not Specified>";
			}
			if(strPharmacy.IsEmpty()) {
				strPharmacy = "<Not Specified>";
			}

			CString strDescription;
			strDescription.Format("Medication: %s, Explanation: %s, Refills: %s, Quantity: %s, Unit: %s, Provider: %s, Pharmacy: %s",
				strMedicationName, strExplanation, strRefills, strQuantity, strUnit, strProvider, strPharmacy);

			//add to our list
			IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
			pRow->PutValue(slcDate, varDate);
			pRow->PutValue(slcCategory, "Prescription");
			pRow->PutValue(slcDescription, _bstr_t(strDescription));
			m_SummaryList->AddRowSorted(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();

		PeekAndPump();

	}NxCatchAll("Error in CPatientSummaryDlg::DisplayPrescriptions");
}

void CPatientSummaryDlg::DisplayFollowUps()
{
	try {

		m_nxstaticStatusLabel.SetWindowText("Loading Follow-Up Tasks...");

		PeekAndPump();

		CWaitCursor pWait;
		
		//there is no license, nor a permission for this

		//Simply display all active to-dos.
		_RecordsetPtr rs = CreateParamRecordset("SELECT Deadline, TodoList.Notes, "
			"dbo.GetTodoAssignToNamesString(TaskID) AS AssignToNames, NoteCatsF.Description AS Category, "
			"UsersT.UserName AS EnteredBy, Task "
			"FROM TodoList "
			"LEFT JOIN NoteCatsF ON TodoList.CategoryID = NoteCatsF.ID "
			"LEFT JOIN UsersT ON TodoList.EnteredBy = UsersT.PersonID "
			"WHERE Done Is Null AND TodoList.PersonID = {INT} "
			"ORDER BY DeadLine DESC", m_nPatientID);
		while(!rs->eof) {
			
			COleDateTime dtDate = AdoFldDateTime(rs, "Deadline");
			CString strTaskDescription = AdoFldString(rs, "Notes", "");
			CString strCategory = AdoFldString(rs, "Category", "");
			CString strTask = AdoFldString(rs, "Task", "");
			CString strAssignTo = AdoFldString(rs, "AssignToNames", "");
			CString strEnteredBy = AdoFldString(rs, "EnteredBy", "");

			if(strTaskDescription.IsEmpty()) {
				strTaskDescription = "<No Description>";
			}
			if(strCategory.IsEmpty()) {
				strCategory = "<No Category>";
			}
			if(strTask.IsEmpty()) {
				strTask = "<Not Specified>";
			}
			if(strAssignTo.IsEmpty()) {
				strAssignTo = "<Not Specified>";
			}
			if(strEnteredBy.IsEmpty()) {
				strEnteredBy = "<Not Specified>";
			}

			CString strDescription;
			strDescription.Format("Category: %s, Method: %s, Description: %s, Assigned To: %s, Entered By: %s",
				strCategory, strTask, strTaskDescription,  strAssignTo, strEnteredBy);

			//add to our list
			IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
			pRow->PutValue(slcDate, _variant_t(dtDate, VT_DATE));
			pRow->PutValue(slcCategory, "Follow-Up Task");
			pRow->PutValue(slcDescription, _bstr_t(strDescription));
			m_SummaryList->AddRowSorted(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();

		PeekAndPump();

	}NxCatchAll("Error in CPatientSummaryDlg::DisplayFollowUps");
}

void CPatientSummaryDlg::DisplayNotes()
{
	try {

		m_nxstaticStatusLabel.SetWindowText("Loading Notes...");

		PeekAndPump();

		CWaitCursor pWait;

		//these is no license for this

		//make sure they have unrestricted read permission
		if(!(GetCurrentUserPermissions(bioPatientNotes) & sptRead)) {
			return;
		}

		//display the past 5 notes
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 5 Notes.Date, Notes.Note, "
			"NoteCatsF.Description AS Category, UsersT.UserName AS EnteredBy "
			"FROM Notes "
			"LEFT JOIN NoteCatsF ON Notes.Category = NoteCatsF.ID "
			"LEFT JOIN UsersT ON Notes.UserID = UsersT.PersonID "
			"WHERE Notes.PersonID = {INT} "
			"ORDER BY Date DESC", m_nPatientID);
		while(!rs->eof) {
			
			COleDateTime dtDate = AdoFldDateTime(rs, "Date");
			CString strNote = AdoFldString(rs, "Note", "");
			CString strCategory = AdoFldString(rs, "Category", "");
			CString strEnteredBy = AdoFldString(rs, "EnteredBy", "");

			if(strNote.IsEmpty()) {
				strNote = "<No Note>";
			}
			if(strCategory.IsEmpty()) {
				strCategory = "<No Category>";
			}
			if(strEnteredBy.IsEmpty()) {
				strEnteredBy = "<Not Specified>";
			}

			CString strDescription;
			strDescription.Format("%s, Category: %s, Entered By: %s", strNote, strCategory, strEnteredBy);

			//add to our list
			IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
			pRow->PutValue(slcDate, _variant_t(dtDate, VT_DATE));
			pRow->PutValue(slcCategory, "Note");
			pRow->PutValue(slcDescription, _bstr_t(strDescription));
			m_SummaryList->AddRowSorted(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();

		PeekAndPump();

	}NxCatchAll("Error in CPatientSummaryDlg::DisplayNotes");
}

void CPatientSummaryDlg::DisplayHistory()
{
	try {

		CWaitCursor pWait;

		//check licensing
		if(!g_pLicense->CheckForLicense(CLicense::lcPatient, CLicense::cflrJustCheckingLicenseNoUI)) {
			return;
		}
		
		//make sure they have unrestricted read permission
		if(!(GetCurrentUserPermissions(bioPatientHistory) & sptRead)) {
			return;
		}

		m_nxstaticStatusLabel.SetWindowText("Loading History...");

		PeekAndPump();

		CWaitCursor pWait2;

		//display the past 5 history entries
		// (j.jones 2008-09-05 12:53) - PLID 30288 - supported MailSentNotesT
		_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 5 MailSent.ServiceDate, MailSent.PathName, "
			"MailSent.Sender, MailSentNotesT.Note, NoteCatsF.Description AS Category "
			"FROM MailSent "
			"LEFT JOIN MailSentNotesT ON MailSent.MailID = MailSentNotesT.MailID "
			"LEFT JOIN NoteCatsF ON MailSent.CategoryID = NoteCatsF.ID "
			"WHERE MailSent.PersonID = {INT} "
			"ORDER BY MailSent.ServiceDate DESC", m_nPatientID);
		while(!rs->eof) {
			
			//for some inexplicable reason, this can be null
			_variant_t varDate = rs->Fields->Item["ServiceDate"]->Value;
			CString strNote = AdoFldString(rs, "Note", "");
			CString strPathName = AdoFldString(rs, "PathName", "");
			CString strCategory = AdoFldString(rs, "Category", "");
			CString strEnteredBy = AdoFldString(rs, "Sender", "");

			if(strCategory.IsEmpty()) {
				strCategory = "<No Category>";
			}
			if(strEnteredBy.IsEmpty()) {
				strEnteredBy = "<Not Specified>";
			}

			if(strPathName == PATHNAME_OBJECT_CASEHISTORY) {
				strNote = "Case History: " + strNote;
			}
			else if(strPathName == PATHNAME_FORM_REPRODUCTIVE) {
				strNote = "IVF Form: " + strNote;
			}
			else if(strPathName == PATHNAME_OBJECT_ELIGIBILITY_REQUEST) {
				strNote = "Eligibility Request: " + strNote;
			}
			else if(strPathName == PATHNAME_OBJECT_ELIGIBILITY_RESPONSE) {
				strNote = "Eligibility Response: " + strNote;
			}

			CString strHistoryDescription = strNote;
			if(strHistoryDescription.IsEmpty()) {
				strHistoryDescription = strPathName;
				if(strHistoryDescription.IsEmpty()) {
					strHistoryDescription = "<No Description>";
				}
			}

			CString strDescription;
			strDescription.Format("%s, Category: %s, Entered By: %s", strHistoryDescription, strCategory, strEnteredBy);

			//add to our list
			IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
			pRow->PutValue(slcDate, varDate);
			pRow->PutValue(slcCategory, "History");
			pRow->PutValue(slcDescription, _bstr_t(strDescription));
			m_SummaryList->AddRowSorted(pRow, NULL);

			rs->MoveNext();
		}
		rs->Close();

		PeekAndPump();

	}NxCatchAll("Error in CPatientSummaryDlg::DisplayHistory");
}

void CPatientSummaryDlg::DisplayTracking()
{
	try {

		CWaitCursor pWait;

		//check licensing
		if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcNexTrak, CLicense::cflrSilent)) {
			return;
		}

		//make sure they have unrestricted read permission
		if(!(GetCurrentUserPermissions(bioPatientTracking) & sptRead)) {
			return;
		}

		m_nxstaticStatusLabel.SetWindowText("Loading Tracking Steps...");

		PeekAndPump();

		CWaitCursor pWait2;

		//find all active ladders
		_RecordsetPtr rs = CreateParamRecordset("SELECT dbo.CalcProcInfoName(ProcInfoT.ID) AS Procedures, "
			"LaddersT.ID, LaddersT.Name AS LadderName "
			"FROM ProcInfoT "
			"INNER JOIN LaddersT ON ProcInfoT.ID = LaddersT.ProcInfoID "
			"INNER JOIN LadderStatusT ON LaddersT.Status = LadderStatusT.ID "
			"INNER JOIN PicT ON ProcInfoT.ID = PicT.ProcInfoID "
			"WHERE LaddersT.ID Is Not Null "
			"AND LaddersT.Status IN (SELECT ID FROM LadderStatusT WHERE IsActive = 1) "
			"AND PicT.IsCommitted = 1 AND ProcInfoT.PatientID = {INT}", m_nPatientID);

		while(!rs->eof) {

			long nLadderID = AdoFldLong(rs, "ID");
			CString strProcedures = AdoFldString(rs, "Procedures", "");
			CString strLadderName = AdoFldString(rs, "LadderName", "");

			CString strLadderDesc = strProcedures;
			if(strLadderDesc.IsEmpty()) {
				strLadderDesc = strLadderName;
				if(strLadderDesc.IsEmpty()) {
					//should be impossible, but handle it anyways
					strLadderDesc = "<No Ladder Description>";
				}
			}

			//find step information for the last step and next step
			_RecordsetPtr rsSteps = CreateParamRecordset("SELECT "
				"IncompletedStepsQ.ActiveDate, IncompletedStepsQ.StepName AS NextStep, "
				"CompletedStepsQ.CompletedDate, CompletedStepsQ.StepName AS LastStep "
				"FROM LaddersT "
				"LEFT JOIN "
				"(SELECT TOP 1 StepsT.LadderID, StepsT.ActiveDate, StepTemplatesT.StepName "
				"	FROM StepsT "
				"	INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
				"	WHERE StepsT.CompletedDate Is Null AND StepsT.LadderID = {INT} "
				"	ORDER BY StepsT.StepOrder ASC) AS IncompletedStepsQ ON LaddersT.ID = IncompletedStepsQ.LadderID "
				"LEFT JOIN (SELECT TOP 1 StepsT.LadderID, StepsT.CompletedDate, StepTemplatesT.StepName "
				"	FROM StepsT "
				"	INNER JOIN StepTemplatesT ON StepsT.StepTemplateID = StepTemplatesT.ID "
				"	WHERE StepsT.CompletedDate Is Null AND StepsT.LadderID = {INT} "
				"	ORDER BY StepsT.CompletedDate DESC, StepsT.StepOrder DESC) AS CompletedStepsQ ON LaddersT.ID = CompletedStepsQ.LadderID "
				"WHERE LaddersT.ID = {INT}", nLadderID, nLadderID, nLadderID);

			if(!rsSteps->eof) {

				_variant_t varStepName = rsSteps->Fields->Item["LastStep"]->Value;
				_variant_t varDate = rsSteps->Fields->Item["CompletedDate"]->Value;
				if(varStepName.vt == VT_BSTR && varDate.vt == VT_DATE) {
					
					//if either are null, we won't add a "step completed" row,
					//so only add if we have both a step name and date
					
					CString strLastStep = VarString(varStepName);
					COleDateTime dtLastStep = VarDateTime(varDate);

					CString strDescription;
					strDescription.Format("%s, Completed Step: %s", strLadderDesc, strLastStep);

					//add to our list
					IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
					pRow->PutValue(slcDate, _variant_t(dtLastStep, VT_DATE));
					pRow->PutValue(slcCategory, "Tracking Step");
					pRow->PutValue(slcDescription, _bstr_t(strDescription));
					m_SummaryList->AddRowSorted(pRow, NULL);
				}

				varStepName = rsSteps->Fields->Item["NextStep"]->Value;
				varDate = rsSteps->Fields->Item["ActiveDate"]->Value;
				if(varStepName.vt == VT_BSTR) {
					
					//if the name is null, we won't add a "current step" row,
					//but the date may be null (will get converted to today's date),
					//so only add if we have a step name
					
					CString strNextStep = VarString(varStepName);
					COleDateTime dtNextStep = VarDateTime(varDate, COleDateTime::GetCurrentTime());

					CString strDescription;
					strDescription.Format("%s, Current Step: %s", strLadderDesc, strNextStep);

					//add to our list
					IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
					pRow->PutValue(slcDate, _variant_t(dtNextStep, VT_DATE));
					pRow->PutValue(slcCategory, "Tracking Step");
					pRow->PutValue(slcDescription, _bstr_t(strDescription));
					m_SummaryList->AddRowSorted(pRow, NULL);
				}
			}
			rsSteps->Close();

			rs->MoveNext();
		}
		rs->Close();

		PeekAndPump();

	}NxCatchAll("Error in CPatientSummaryDlg::DisplayTracking");
}

//copied from PatientLabsDlg, used in DisplayLabs()
//TES 11/10/2009 - PLID 36260 - This is no longer an enum, but a configurable table, so the description should be loaded along with the ID.
//TES 12/8/2009 - PLID 36512 - AnatomySide is back!
CString CPatientSummaryDlg::GetAnatomySideString(long nSide)
{
	CString strSideName = "";
	try {
		switch(nSide) {
		case asLeft:
			strSideName = "Left"; //Left
			break;
		case asRight:
			strSideName = "Right"; //Right
			break;
		case asNone:
		default:
			strSideName = ""; //(No selection)
			break;
		}
	} NxCatchAll("Error in CPatientSummaryDlg::GetAnatomySideString");
	return strSideName;
}

void CPatientSummaryDlg::DisplayLabs()
{
	try {

		CWaitCursor pWait;

		//check licensing
		if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcLabs, CLicense::cflrSilent)) {
			return;
		}

		//make sure they have unrestricted read permission
		if(!(GetCurrentUserPermissions(bioPatientLabs) & sptRead)) {
			return;
		}

		m_nxstaticStatusLabel.SetWindowText("Loading Labs...");

		PeekAndPump();

		CWaitCursor pWait2;

		// (j.gruber 2008-10-10 10:50) - PLID 31432 - changed to support multi-lab structure
		//display all active labs, plus any labs completed in the past month
		// (z.manning 2008-10-30 11:58) - PLID 31864 - Added to be ordered and type for labs
		//TES 11/10/2009 - PLID 36260 - Replaced AnatomySide with AnatomyQualifierID
		//TES 12/8/2009 - PLID 36512 - AnatomySide
		// (c.haag 2010-12-13 12:10) - PLID 41806 - Use stored functions to get lab completed info
		//TES 8/5/2011 - PLID 44901 - Filter on permissioned locations
		_RecordsetPtr rs = CreateParamRecordset(
			"SELECT LabsT.*, LabAnatomyT.Description AS AnatomicLocation, "
			"UsersT.UserName, "
			"AnatomyQualifiersT.Name AS LocationQualifier, "
			" CASE WHEN (SELECT Count(*) FROM LabResultsT "
			" WHERE LabResultsT.Deleted = 0 AND LabResultsT.LabID = LabsT.ID) > 1 THEN '<Multiple>' ELSE "
			" (SELECT LabResultFlagsT.Name FROM LabResultFlagsT LEFT JOIN LabResultsT "
			" ON LabResultFlagsT.ID = LabResultsT.FlagID WHERE LabResultsT.Deleted = 0 AND LabResultsT.LabID = LabsT.ID) END AS ResultFlagName "
			"FROM ("
			"SELECT LabsT.ID, LabsT.FormNumberTextID, LabsT.Specimen, LabsT.InputDate, LabsT.AnatomyID, LabsT.AnatomyQualifierID, "
			" LabsT.AnatomySide, "
			"dbo.GetLabCompletedDate(LabsT.ID) AS LabCompletedDate, "
			"dbo.GetLabCompletedBy(LabsT.ID) AS LabCompletedBy, "
			"LabsT.Type, LabsT.ToBeOrdered, "
			//(e.lally 2010-10-19) PLID 31830 - get today's date from the server
			"GetDate() AS TodaysDate "
			"FROM LabsT "
			"WHERE LabsT.PatientID = {INT} AND LabsT.Deleted = 0 AND {SQLFRAGMENT} "
			") LabsT "
			"LEFT JOIN LabAnatomyT ON LabAnatomyT.ID = LabsT.AnatomyID "
			"LEFT JOIN AnatomyQualifiersT ON AnatomyQualifiersT.ID = LabsT.AnatomyQualifierID "
			"LEFT JOIN UsersT ON LabsT.LabCompletedBy = UsersT.PersonID "
			"WHERE (LabsT.LabCompletedDate Is Null OR LabsT.LabCompletedDate > DATEADD(month, -1, GetDate())) "
			"ORDER BY LabsT.InputDate DESC", m_nPatientID, GetAllowedLocationClause_Param("LabsT.LocationID"));
		while(!rs->eof) {
			
			long nLabID = AdoFldLong(rs, "ID");
			CString strFormNumberTextID = AdoFldString(rs, "FormNumberTextID", "");
			CString strSpecimen = AdoFldString(rs, "Specimen", "");
			CString strResult = AdoFldString(rs, "ResultFlagName", "Pending");
			COleDateTime dtInput = AdoFldDateTime(rs, "InputDate");
			_variant_t vLabCompletedDate = rs->Fields->Item["LabCompletedDate"]->Value;
			BOOL bLabComplete = vLabCompletedDate.vt == VT_DATE;

			//(e.lally 2010-10-19) PLID 31830 - get today's date from the server
			COleDateTime dtTodaysDate = AdoFldDateTime(rs, "TodaysDate");

			// (z.manning 2008-10-30 11:58) - PLID 31864 - Use to be ordered instead of anatomic location
			// on non-biopsy labs
			LabType eType = (LabType)AdoFldByte(rs->GetFields(), "Type");
			CString strTypeDesc;
			if(eType == ltBiopsy) {
				//Assemble the anatomic location(s)
				CString strAnatomicLocation = AdoFldString(rs, "AnatomicLocation", "");
				//TES 11/10/2009 - PLID 36260 - Replaced AnatomySide with AnatomyQualifierID
				//TES 12/8/2009 - PLID 36512 - Restored AnatomySide
				AnatomySide as = (AnatomySide)AdoFldLong(rs, "AnatomySide",0);
				CString strQualifier = AdoFldString(rs, "LocationQualifier", "");
				// (z.manning 2010-04-30 17:52) - PLID 37553 - We now have a function to format this properly
				strTypeDesc = ::FormatAnatomicLocation(strAnatomicLocation, strQualifier, as);
			}
			else {
				strTypeDesc = AdoFldString(rs->GetFields(), "ToBeOrdered", "");
			}

			if(strSpecimen.IsEmpty()) {
				strSpecimen = "<No Specimen>";
			}

			if(strResult.IsEmpty()) {
				strResult = "Pending";
			}
			
			//Assemble the description for the lab
			CString strLabDescription;
			strLabDescription.Format("%s-%s - %s, Result: %s", strFormNumberTextID, strSpecimen, strTypeDesc, strResult);

			//find the most recently completed step
			_RecordsetPtr rsLastStep = CreateParamRecordset("SELECT TOP 1 "
				"LabStepsT.StepOrder, LabStepsT.Name, LabStepsT.StepCompletedDate "
				"FROM LabStepsT "
				"WHERE LabID = {INT} AND StepCompletedDate Is Not Null "
				"ORDER BY StepCompletedDate DESC, StepOrder DESC", nLabID);

			long nLastStepOrder = -1;
			//(e.lally 2010-10-19) PLID 31830 - Track if we have a last step entry
			bool bHasLastStep = false;
			if(!rsLastStep->eof) {
				bHasLastStep = true;
				nLastStepOrder = AdoFldLong(rsLastStep, "StepOrder", -1);
				CString strStepName = AdoFldString(rsLastStep, "Name", "");
				COleDateTime dtStepDate = AdoFldDateTime(rsLastStep, "StepCompletedDate");

				CString strDescription;
				strDescription.Format("%s, Step Completed: %s", strLabDescription, strStepName);

				//add to our list
				IRowSettingsPtr pRow = m_SummaryList->GetNewRow();
				pRow->PutValue(slcDate, _variant_t(dtStepDate, VT_DATE));
				pRow->PutValue(slcCategory, "Lab");
				pRow->PutValue(slcDescription, _bstr_t(strDescription));
				m_SummaryList->AddRowSorted(pRow, NULL);
			}
			rsLastStep->Close();

			//now find the current step, but unlike tracking we can't find the first
			//uncompleted step, because completing a lab step doesn't mark prior steps
			//as completed, so we need to find the next step in order, using a new recordset
			_RecordsetPtr rsNextStep = CreateParamRecordset("SELECT TOP 1 "
				"LabStepsT.Name, GetDate() AS TodaysDate "
				"FROM LabStepsT "
				"WHERE LabID = {INT} AND StepCompletedDate Is Null "
				"AND StepOrder > {INT} "
				"ORDER BY StepOrder ASC", nLabID, nLastStepOrder);

			//(e.lally 2010-10-19) PLID 31830 - Set the default description to the lab description
			CString strDescription = strLabDescription;
			//(e.lally 2010-10-19) PLID 31830 - Track if we have a current step record
			bool bHasCurrentStep = false;
			if(!rsNextStep->eof) {
				bHasCurrentStep = true;
				CString strStepName = AdoFldString(rsNextStep, "Name", "");
				//(e.lally 2010-10-19) PLID 31830 - Update description to include current step
				strDescription.Format("%s, Current Step: %s", strLabDescription, strStepName);
			}
			//(e.lally 2010-10-19) PLID 31830 - If it has a current step or no steps at all, add to our list
			if(bHasCurrentStep == true || (bHasCurrentStep == false && bHasLastStep == false)){
				IRowSettingsPtr pRow = m_SummaryList->GetNewRow();				
				//we don't have a step date, so always show today's date from the server
				pRow->PutValue(slcDate, _variant_t(dtTodaysDate, VT_DATE));
				pRow->PutValue(slcCategory, "Lab");
				pRow->PutValue(slcDescription, _bstr_t(strDescription));
				m_SummaryList->AddRowSorted(pRow, NULL);
			}

			rsNextStep->Close();

			rs->MoveNext();
		}
		rs->Close();

		PeekAndPump();

	}NxCatchAll("Error in CPatientSummaryDlg::DisplayLabs");
}

//GetHelpText() builds the text to be used for the help and do not show me again messages.
//Right now the text is static, but if we ever add preferences to control the ranges
//of data shown, we would want to dynamically tweak the wording of this text.
CString CPatientSummaryDlg::GetHelpText()
{
	CString strMessage;

	try {

		//keep the info. as simple as possible, for example we don't advertise that we show EMNs in the future
		//if they exist, just give them the information they need to identify where the timeline cuts off
		// (j.gruber 2010-06-15 14:19) - PLID 26363 - this is just for the bottom list

		strMessage = "The Patient Summary bottom screen displays the following information for the current patient:\n\n"			
			" - All future Appointments, and the last three appointments that have occurred within the last three months.\n"
			" - All unpaid Bills, and the last three bills that have been fully paid off.\n"
			" - All unapplied Payments, and the last three payments that have been fully applied.\n"
			" - All active unbilled quotes, and incompleted packages.\n"
			" - The last five notes entered for the patient.\n"
			" - The last five entries in the History tab.\n"			
			" - The last five EMNs for the patient.\n"
			" - All unfinished follow-up tasks for the patient.\n"			
			" - Prescriptions written for the last three patient visits that had prescriptions created.\n"						
			" - The most recently completed step, and current step, for all active tracking ladders.\n"
			" - The most recently completed step, and current step, for all active labs, "
			"and the last step completed for any labs completed within the past month.";

		return strMessage;

	}NxCatchAll("Error in CPatientSummaryDlg::GetHelpText");

	return strMessage;
}

void CPatientSummaryDlg::OnBtnSummaryHelp() 
{
	try {

		AfxMessageBox(GetHelpText());

	}NxCatchAll("Error in CPatientSummaryDlg::OnBtnSummaryHelp");
}

void CPatientSummaryDlg::OnBtnSummaryClose() 
{
	try {

		CNxDialog::OnOK();

	}NxCatchAll("Error in CPatientSummaryDlg::OnBtnSummaryClose");
}

// (j.jones 2010-05-26 09:51) - PLID 38508 - loads the info. at the top of the screen
// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen - not used anymore
/*void CPatientSummaryDlg::FillDialogFields()
{
	try {

		//load the fixed fields at the top of the dialog, do so independently
		//of the other loading on this screen

		CWaitCursor pWait;

		m_nxstaticStatusLabel.SetWindowText("Loading...");

		//last appointment info
		{
			BOOL bCanSeeAppointments = TRUE;
			CString strLastAppointment = "<Not Available>";

			//check licensing
			if(!g_pLicense || !g_pLicense->CheckSchedulerAccess_Any(CLicense::cflrSilent)) {
				bCanSeeAppointments = FALSE;
				strLastAppointment = "<Not Available>";
			}

			//make sure they have unrestricted view permission
			if(bCanSeeAppointments && !(GetCurrentUserPermissions(bioAppointment) & sptView)) {
				bCanSeeAppointments = FALSE;
				strLastAppointment = "<Not Available>";
			}

			if(bCanSeeAppointments) {

				//find the last non-cancelled, non no-show appointment
				_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 "
					"ResBasicQ.StartTime, "
					"dbo.GetResourceString(ResBasicQ.ID) AS Resource, "
					"CASE WHEN ResBasicQ.AptPurposeName <> '' THEN AptTypeT.Name + ' - ' + ResBasicQ.AptPurposeName ELSE AptTypeT.Name END AS Purpose "
					"FROM (SELECT AppointmentsT.ID, AppointmentsT.PatientID, AppointmentsT.AptTypeID, "
					"	dbo.GetPurposeString(AppointmentsT.ID) AS AptPurposeName, "
					"	AppointmentsT.StartTime, AppointmentsT.Status, AppointmentsT.ShowState "
					"	FROM AppointmentsT "
					"	LEFT JOIN PersonT ON AppointmentsT.PatientID = PersonT.ID "
					"	WHERE AppointmentsT.Status <> 4 AND AppointmentsT.ShowState <> 3) ResBasicQ "
					"INNER JOIN AptTypeT ON ResBasicQ.AptTypeID = AptTypeT.ID "
					"WHERE PatientID = {INT} "
					"AND StartTime < GetDate() "
					"ORDER BY StartTime DESC", m_nPatientID);

				if(!rs->eof) {

					COleDateTime dtTime = AdoFldDateTime(rs, "StartTime");
					CString strResource = AdoFldString(rs, "Resource", "");
					CString strPurpose = AdoFldString(rs, "Purpose", "");

					if(strPurpose.IsEmpty()) {
						strPurpose = "<No Type, No Purpose>";
					}

					strLastAppointment.Format("%s - %s, Resource: %s", FormatDateTimeForInterface(dtTime, DTF_STRIP_SECONDS, dtoDateTime), strPurpose, strResource);
				}
				else {
					strLastAppointment = "<No Previous Appointment>";
				}
				rs->Close();
			}

			SetDlgItemText(IDC_EDIT_LAST_APPOINTMENT_INFO, strLastAppointment);
		}

		BOOL bCanSeeBills = TRUE;

		//check billing licensing & perms
		if(!g_pLicense || !g_pLicense->CheckForLicense(CLicense::lcBill, CLicense::cflrSilent)) {
			bCanSeeBills = FALSE;
		}

		//make sure they have unrestricted read permission
		if(bCanSeeBills && !(GetCurrentUserPermissions(bioPatientBilling) & sptRead)) {
			bCanSeeBills = FALSE;
		}

		//last bill's diagnosis codes
		{						
			CString strLastBilledDiagCodes = "<Not Available>";

			if(bCanSeeBills) {

				//find the last bill created for the patient, and show its diagnosis codes
				_RecordsetPtr rs = CreateParamRecordset("SELECT TOP 1 "
					"dbo.GetDiagListFromBill(BillsT.ID) AS DiagList "
					"FROM BillsT "
					"WHERE BillsT.Deleted = 0 AND EntryType = 1 "
					"AND BillsT.PatientID = {INT} "
					"ORDER BY BillsT.Date DESC, BillsT.ID DESC ",
					m_nPatientID);

				if(rs->eof) {
					strLastBilledDiagCodes = "<No Previous Bill>";
				}
				else {
					
					strLastBilledDiagCodes = AdoFldString(rs, "DiagList", "");

					if(strLastBilledDiagCodes.IsEmpty()) {
						strLastBilledDiagCodes = "<No Diagnosis Codes On Last Bill>";
					}
				}
				rs->Close();
			}

			SetDlgItemText(IDC_EDIT_LAST_DIAG_CODES, strLastBilledDiagCodes);
		}

		//account balances
		{						
			CString strPatientBalance = "<Not Available>";
			CString strInsuranceBalance = "<Not Available>";

			if(bCanSeeBills) {

				_RecordsetPtr rs = CreateParamRecordset("SELECT "
					"Convert(money, Coalesce(TotalPatientCharges, 0) - Coalesce(TotalPatientPayments, 0)) AS PatientBalance, "
					"Convert(money, Coalesce(TotalInsuranceCharges, 0) - Coalesce(TotalInsurancePayments, 0)) AS InsuranceBalance "
					"FROM PersonT "
					"LEFT JOIN ("
					"	SELECT BillsT.PatientID, Sum(Round(Convert(money,ChargeRespT.Amount),2)) AS TotalPatientCharges "
					"	FROM BillsT "
					"	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
					"	INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
					"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"	WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
					"	AND LineItemT.Type = 10 AND BillsT.EntryType = 1 "
					"	AND (ChargeRespT.InsuredPartyID Is Null OR ChargeRespT.InsuredPartyID = -1) "
					"	GROUP BY BillsT.PatientID "
					"	) AS PatientChargeTotalQ ON PersonT.ID = PatientChargeTotalQ.PatientID "
					"LEFT JOIN ("
					"	SELECT BillsT.PatientID, Sum(Round(Convert(money,ChargeRespT.Amount),2)) AS TotalInsuranceCharges "
					"	FROM BillsT "
					"	INNER JOIN ChargesT ON BillsT.ID = ChargesT.BillID "
					"	INNER JOIN ChargeRespT ON ChargesT.ID = ChargeRespT.ChargeID "
					"	INNER JOIN LineItemT ON ChargesT.ID = LineItemT.ID "
					"	WHERE BillsT.Deleted = 0 AND LineItemT.Deleted = 0 "
					"	AND LineItemT.Type = 10 AND BillsT.EntryType = 1 "
					"	AND ChargeRespT.InsuredPartyID Is Not Null AND ChargeRespT.InsuredPartyID <> -1 "
					"	GROUP BY BillsT.PatientID "
					"	) AS InsuranceChargeTotalQ ON PersonT.ID = InsuranceChargeTotalQ.PatientID "
					"LEFT JOIN ("
					"	SELECT LineItemT.PatientID, Sum(LineItemT.Amount) AS TotalPatientPayments "
					"	FROM PaymentsT "
					"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					"	WHERE LineItemT.Deleted = 0 "
					"	AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 "
					"	AND (PaymentsT.InsuredPartyID Is Null OR PaymentsT.InsuredPartyID = -1) "
					"	GROUP BY LineItemT.PatientID "
					"	) AS PatientPaymentTotalQ ON PersonT.ID = PatientPaymentTotalQ.PatientID "
					"LEFT JOIN ("
					"	SELECT LineItemT.PatientID, Sum(LineItemT.Amount) AS TotalInsurancePayments "
					"	FROM PaymentsT "
					"	INNER JOIN LineItemT ON PaymentsT.ID = LineItemT.ID "
					"	WHERE LineItemT.Deleted = 0 "
					"	AND LineItemT.Type >= 1 AND LineItemT.Type <= 3 "
					"	AND PaymentsT.InsuredPartyID Is Not Null AND PaymentsT.InsuredPartyID <> -1 "
					"	GROUP BY LineItemT.PatientID "
					"	) AS InsurancePaymentTotalQ ON PersonT.ID = InsurancePaymentTotalQ.PatientID "
					"WHERE PersonT.ID = {INT}", m_nPatientID);

				COleCurrency cyPatBal = COleCurrency(0,0);
				COleCurrency cyInsBal = COleCurrency(0,0);

				if(!rs->eof) {
					cyPatBal = AdoFldCurrency(rs, "PatientBalance", COleCurrency(0,0));
					cyInsBal = AdoFldCurrency(rs, "InsuranceBalance", COleCurrency(0,0));
				}
				rs->Close();

				strPatientBalance = FormatCurrencyForInterface(cyPatBal);
				strInsuranceBalance = FormatCurrencyForInterface(cyInsBal);
			}

			SetDlgItemText(IDC_EDIT_PATIENT_BALANCE, strPatientBalance);
			SetDlgItemText(IDC_EDIT_INSURANCE_BALANCE, strInsuranceBalance);
		}

		PeekAndPump();

	}NxCatchAll("Error in CPatientSummaryDlg::FillDialogFields");
}*/

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::OnBnClickedBtnSummaryConfigure()
{
	try {
		CPatientSummaryConfigDlg dlg(&m_mapCustomFields, this);
		long nResult = dlg.DoModal();

		if (nResult == IDOK) {
			LoadConfigValues();
			LoadTopList();
		}
	}NxCatchAll(__FUNCTION__);
}
BEGIN_EVENTSINK_MAP(CPatientSummaryDlg, CNxDialog)
	ON_EVENT(CPatientSummaryDlg, IDC_PAT_INFO_LIST, 19, CPatientSummaryDlg::LeftClickPatInfoList, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
END_EVENTSINK_MAP()

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::LeftClickPatInfoList(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try {
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);

		if (pRow) {

			if (pRow->GetCellLinkStyle(nCol) == NXDATALIST2Lib::dlLinkStyleTrue) {
				
				CString strTabLink;

				switch (nCol) {
	
					case dlcLeft:
						strTabLink = VarString(pRow->GetValue(dlcLeftLink), "");
					break;

					case dlcRight:
						strTabLink = VarString(pRow->GetValue(dlcRightLink), "");
					break;
				}

				if (!strTabLink.IsEmpty()) {
					MoveToTab(strTabLink);
				}
			}
		}
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2010-06-15 14:11) - PLID 26363 - added configurable summary screen
void CPatientSummaryDlg::MoveToTab(CString strTabLink)
{
	//first close the dialog
	//(e.lally 2012-05-01) PLID 49635 - Use IDYES to indicate we closed in a non OK/Cancel situation
	EndDialog(IDYES);

	//now move to the correct tab
	short nTab;
	if (strTabLink == "tabG1") {		
		nTab = PatientsModule::General1Tab;
	}
	else if (strTabLink == "tabG2") {
		nTab = PatientsModule::General2Tab;
	}
	else if (strTabLink == "tabCustom") {
		nTab = PatientsModule::CustomTab;
	}
	else if (strTabLink == "tabMed") {
		nTab = PatientsModule::MedicationTab;
	}
	else if (strTabLink == "tabAppt") {
		nTab = PatientsModule::AppointmentTab;
	}
	else if (strTabLink == "tabInsurance") {
		nTab = PatientsModule::InsuranceTab;
	}
	else if (strTabLink == "tabBill") {
		nTab = PatientsModule::BillingTab;
	}
	else {
		ASSERT(FALSE);
		nTab = PatientsModule::General1Tab;
	}

	// (j.gruber 2011-06-17 13:01) - PLID 43989 - this can be opened from other modules now
	if (GetMainFrame()->FlipToModule(PATIENT_MODULE_NAME)) {
		CNxTabView *pView = (CNxTabView *)GetMainFrame()->GetOpenView(PATIENT_MODULE_NAME);
		if (pView) 
		{	
			if(pView->GetActiveTab() != nTab) {
				pView->SetActiveTab(nTab);
			}
			
			pView->UpdateView();
		}		
	}

}