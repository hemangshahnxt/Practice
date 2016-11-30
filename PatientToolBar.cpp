#include "stdafx.h"
#include "resource.h"
#include "PatientToolBar.h"
#include "ChildFrm.h"
#include "GlobalUtils.h"
#include "NxStandard.h"
#include "PracProps.h"
#include "FinancialDlg.h"
#include "MainFrm.h"
#include "NxTabView.h"
#include "GlobalDataUtils.h"
#include "PracticeRc.h"
#include "NxGdiPlusUtils.h"
#include "OHIPUtils.h"
#include "InternationalUtils.h"
#include "AlbertaHLINKUtils.h"

using namespace Gdiplus;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/*
#define BUTTON1 0
#define BUTTON2 1
#define BUTTON3 2
#define BUTTON4 3
#define COMBO	5
#define RIGHT1	7
#define RIGHT2	8
#define RIGHT3	9
*/

// (a.walling 2008-08-20 15:43) - PLID 29642 - Enumerate toolbar button indexes
enum EPatientToolbarButtons {
	tbSearch = 0,
	tbLeft,
	tbCombo,
	tbRight,
	tbHistory, // (a.walling 2010-05-21 10:12) - PLID 17768
	tbCount,
};

using namespace ADODB;

// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2 - Everything here now uses a DL2; I did not comment every change.
using namespace NXDATALIST2Lib;

//(e.lally 2012-02-06) PLID 40174 - Removed local abs() macro define

// (a.walling 2008-08-20 15:43) - PLID 29642 - Add handlers for CustomDraw, EraseBkgnd, MouseMove, and MouseLeave
// (a.walling 2008-08-21 14:09) - PLID 31056 - Added ON_WM_LBUTTONUP to fix drawing issue with tooltips
// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing (NcPaint)
BEGIN_MESSAGE_MAP(CPatientToolBar, CToolBar)
	//{{AFX_MSG_MAP(CPatientToolBar)
	ON_NOTIFY_REFLECT_EX(NM_CUSTOMDRAW, OnCustomDraw)
	ON_WM_ENABLE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_LBUTTONUP()
	ON_WM_NCPAINT()
	ON_WM_RBUTTONDOWN()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CPatientToolBar, CToolBar)
	//{{AFX_EVENTSINK_MAP(CPatientToolBar)
	ON_EVENT(CPatientToolBar, IDW_COMBO, 16 /* SelChosen */, OnSelChosenPatientCombo, VTS_DISPATCH)
	ON_EVENT(CPatientToolBar, IDW_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedPatientCombo, VTS_I2)
	ON_EVENT(CPatientToolBar, IDW_COMBO, 20 /* TrySetSelFinished */, OnTrySetSelFinishedPatientCombo, VTS_I4 VTS_I4)
	ON_EVENT(CPatientToolBar, IDW_COMBO, 17 /* ColumnClicking */, OnColumnClickingPatientCombo, VTS_I2 VTS_PBOOL)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

// (a.walling 2008-08-20 15:42) - PLID 29642 - Prepare theme variables
CPatientToolBar::CPatientToolBar()
 : m_uxtTheme(*(new UXTheme))
{
	// (a.walling 2012-01-04 17:13) - PLID 47324 - Disable active accessibility for this window due to module state issues (see this item notes for more info)
	m_bEnableActiveAccessibility = false;

	m_PatientStatusFromSchedule = -1;
	m_bChangedInScheduler = false;
	m_PatientIDFromScheduler = -1;
	m_pFont = NULL;

	m_toolBarCombo = NULL;
	
	m_strFilterFrom.Empty();
	m_strFilterWhere.Empty();
	m_strFilterString.Empty();

	m_bThemeInit = FALSE;

	m_nLastMouseOverIx = LONG_MIN;
	m_bTrackingMouse = FALSE;

	//m_bEnableActiveAccessibility = false;
}

// (a.walling 2008-08-20 15:42) - PLID 29642 - Cleanup theme data
CPatientToolBar::~CPatientToolBar()
{
	if (m_pFont)
		delete m_pFont;

	m_searchButtonFont.DeleteObject();

	m_uxtTheme.CloseThemeData();
	delete &m_uxtTheme;
}


BOOL CPatientToolBar::PreTranslateMessage(MSG* pMsg)
{
	// Note: there is only one control in the toolbar that accepts keyboard input,
	// this is the combobox.

	if (pMsg->message == WM_KEYDOWN)
	{
		// user hit ENTER
		if (pMsg->wParam == VK_RETURN && GetKeyState(VK_RETURN) < 0)
		{
/*DRT 5/28/2004 - PLID 12649 - This isn't even used (c.haag commented out the return statements a long
//time ago), but it has still been pulling out a value.  Not only is this an unnecessary thing to do, 
//if you select nothing in the datalist, this will crash practice.
			// extract the text, update combobox lists, and do the search
			CString s1;
			s1 = CString(m_toolBarCombo->GetCurSel()->GetValue(ptbcFullName).bstrVal);

			if(s1.GetLength()) {
				// CAH 1/31 -- Prevents proper keyboard navigation
				//return TRUE;  // key processed
			} else {
				//MessageBeep(MB_ICONEXCLAMATION);
				// CAH 1/31 -- Prevents proper keyboard navigation
				//return TRUE;  // key processed
			}
*/		}

		// user hit ESC
		if (pMsg->wParam == VK_ESCAPE && GetKeyState(VK_ESCAPE) < 0)
		{
			// CAH 1/31 -- Prevents proper keyboard navigation
			//return TRUE; // key processed
		}

		// user hit DOWN-ARROW
		if (pMsg->wParam == VK_DOWN && GetKeyState(VK_DOWN) < 0)
		{
			// CAH 1/31 -- Prevents proper keyboard navigation
			//return TRUE; // key processed
		}
	}
	return 	CToolBar::PreTranslateMessage(pMsg);
}

CString CPatientToolBar::GetDefaultFilter()
{
	CString Ans;
	CString strWhereAnd;

	long nDefaultPatientModuleSearch = GetRemotePropertyInt("DefaultPatientModuleSearch",0,0,GetCurrentUserName(),TRUE);

	long nDefaultPatientModuleInclude = GetRemotePropertyInt("DefaultPatientModuleInclude",1,0,GetCurrentUserName(),TRUE);

	// See if we want only active patients
	if (nDefaultPatientModuleInclude == 1) {
		// Add " WHERE (Archived = false)"
		Ans += strWhereAnd + CString("Archived = 0");
		strWhereAnd = " AND ";
	}

	// See if we want Patients, Prospects, or All
	// (d.moore 2007-05-02 14:32) - PLID 23602 - 'CurrentStatus = 3' was previously used for both
	//  patients and prospects. It should only be used for patients.
	if(nDefaultPatientModuleSearch == 1) {
		// Add " AND (CurrentStatus = 1 OR CurrentStatus = 3)"
		Ans += strWhereAnd + CString("(CurrentStatus = 1 OR CurrentStatus = 3)");
		strWhereAnd = " AND ";
	} else if (nDefaultPatientModuleSearch == 2) {
		// Add " AND (CurrentStatus = 2 OR CurrentStatus = 3)"
		Ans += strWhereAnd + CString("(CurrentStatus = 2)");
		strWhereAnd = " AND ";
	} else {
		// Add nothing
		Ans += strWhereAnd + CString("(CurrentStatus <> 4)");
		strWhereAnd = "AND ";
	}

	return Ans;
}

void CPatientToolBar::PopulateColumns()
{	
	// Remove all existing columns
	while (m_toolBarCombo->GetColumnCount())
	{
		m_toolBarCombo->RemoveColumn(0);
	}

	// Now set up the columns
	short nLastNameCol = GetLastNameColumn();
	short nFirstNameCol = GetFirstNameColumn();
	short nMiddleNameCol = GetMiddleNameColumn();
	short nPatientIDCol = GetPatientIDColumn();
	short nBirthDateCol = GetBirthDateColumn();
	short nSSNCol = GetSSNColumn();
	// (j.jones 2010-05-04 10:55) - PLID 32325 - added OHIP Health Card Column
	short nOHIPHealthCardCol = GetOHIPHealthCardColumn();
	//m.hancock - 5/8/2006 - PLID 20462 - Add the company name as an option to the new patient toolbar
	short nCompanyCol = GetCompanyColumn();

	// (j.gruber 2010-10-04 14:28) - PLID 40415 - Security Group Column
	short nSecurtyGroupCol = GetSecurityGroupColumn();

	// (a.walling 2010-05-26 16:00) - PLID 38906 - We can save a lot of memory by calculating the 'full name' in the datalist's display column
	/*
	_bstr_t bstrDisplayField = "[Last] + ', ' + [First] + ' ' + [Middle]";
	// (z.manning 2008-11-12 11:02) - PLID 31129 - We now have a preference to inlcude the ID in the
	// patient combo's display field.
	if(GetRemotePropertyInt("PatientToolbarShowID", 1, 0, "<None>", true) == 1) {
		bstrDisplayField += " + ' (' + convert(nvarchar(50),ComboQ.[PatientID]) + ')'";
	}
	*/

	// (c.haag 2004-04-21 14:59) - Hidden columns will stay hard coded.
	IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(ptbcPersonID, _T("PersonID"), _T("PersonID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
	//IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(ptbcFullName, _T(bstrDisplayField), _T("Full Name"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
	IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(ptbcCurrentStatus, _T("CurrentStatus"), _T("CurrentStatus"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;

	// (j.gruber 2011-07-22 15:46) - PLID 44118 - instrt forecolor field
	IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(ptbcForeColor, _T("ForeColor"), _T("ForeColor"), 0, csVisible|csFixedWidth)))->FieldType = cftSetRowForeColor;

	// (c.haag 2004-04-21 14:59) - Now deal with dynamic columns that the user may hide or move
	for (short i=ptbcHardColumnCount; i < ptbcHardColumnCount + 9; i++)
	{
		if (i == nLastNameCol)
		{
			if (GetRemotePropertyInt("PtTB_Last Name", 1, 0, GetCurrentUserName(), true) > 0)
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("Last"), _T("Last Name"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
			else
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("Last"), _T("Last Name"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		}
		else if (i == nFirstNameCol)
		{
			if (GetRemotePropertyInt("PtTB_First Name", 2, 0, GetCurrentUserName(), true) > 0)
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("First"), _T("First Name"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
			else
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("First"), _T("First Name"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		}
		else if (i == nMiddleNameCol)
		{
			if (GetRemotePropertyInt("PtTB_Middle Name", 3, 0, GetCurrentUserName(), true) > 0)
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("Middle"), _T("Middle Name"), 7, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
			else
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("Middle"), _T("Middle Name"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		}
		else if (i == nPatientIDCol)
		{
			if (GetRemotePropertyInt("PtTB_Patient ID", 4, 0, GetCurrentUserName(), true) > 0)
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("ComboQ.PatientID"), _T("Patient ID"), 14, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
			else
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("ComboQ.PatientID"), _T("Patient ID"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		}
		else if (i == nBirthDateCol)
		{
			// (j.jones 2007-08-10 15:26) - PLID 27047 - forced this column to be a date value
			//TES 1/18/2010 - PLID 36895 - Need to hide this for blocked patients
			if (GetRemotePropertyInt("PtTB_Birth Date", 5, 0, GetCurrentUserName(), true) > 0)
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("CASE WHEN BlockedPatientsQ.PatientID Is Null THEN BirthDate ELSE Null END"), _T("Birth Date"), 15, csVisible|csWidthPercent)))->FieldType = cftDateNatural;
			else
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("CASE WHEN BlockedPatientsQ.PatientID Is Null THEN BirthDate ELSE Null END"), _T("Birth Date"), 0, csVisible|csFixedWidth)))->FieldType = cftDateNatural;
		}
		else if (i == nSSNCol)
		{
			//TES 1/18/2010 - PLID 36895 - Need to hide this for blocked patients
			// (j.jones 2010-05-04 11:14) - PLID 32375 - hide when UseOHIP() is enabled
			// (f.dinatale 2010-10-11) - PLID 33753 - If the user does not have permission to read unmasked SSNs, don't show the column.
			// (j.jones 2010-11-08 13:47) - PLID 39620 - hide if Alberta is in use
			if (!UseOHIP() && !UseAlbertaHLINK() && GetRemotePropertyInt("PtTB_SSN", 6, 0, GetCurrentUserName(), true) > 0 && CheckCurrentUserPermissions(bioPatientSSNMasking, sptRead, FALSE, 0, TRUE) && CheckCurrentUserPermissions(bioPatientSSNMasking, sptDynamic0, FALSE, 0, TRUE)) {
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("CASE WHEN BlockedPatientsQ.PatientID Is Null THEN SocialSecurity ELSE Null END"), _T("SS #"), 18, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
			} else {
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("CASE WHEN BlockedPatientsQ.PatientID Is Null THEN SocialSecurity ELSE Null END"), _T("SS #"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
			}
		}
		//m.hancock - 5/8/2006 - PLID 20462 - Add the company name as an option to the new patient toolbar
		else if (i == nCompanyCol)
		{
			//TES 1/18/2010 - PLID 36895 - Need to hide this for blocked patients			
			if (GetRemotePropertyInt("PtTB_Company or School", -7, 0, GetCurrentUserName(), true) > 0)
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("CASE WHEN BlockedPatientsQ.PatientID Is Null THEN Company ELSE Null END"), _T("Company"), 18, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
			else
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("CASE WHEN BlockedPatientsQ.PatientID Is Null THEN Company ELSE Null END"), _T("Company"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		}
		// (j.jones 2010-05-04 11:16) - PLID 32325 - added OHIP Health Card column
		else if (i == nOHIPHealthCardCol)
		{
			CString strOHIPHealthTitle = "Health Card Num.";
			if(UseOHIP()) {
				//if enabled, get the actual name of the custom field
				long nHealthNumberCustomField = GetRemotePropertyInt("OHIP_HealthNumberCustomField", 1, 0, "<None>", true);
				_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM CustomFieldsT WHERE ID = {INT}", nHealthNumberCustomField);
				if(!rs->eof) {
					CString str = AdoFldString(rs, "Name", "");
					str.TrimLeft();
					str.TrimRight();
					if(!str.IsEmpty()) {
						strOHIPHealthTitle = str;
					}
				}
				rs->Close();
			}
			// (j.jones 2010-11-08 13:48) - PLID 39620 - supported Alberta
			else if(UseAlbertaHLINK()) {
				//if enabled, get the actual name of the custom field
				long nHealthNumberCustomField = GetRemotePropertyInt("Alberta_PatientULICustomField", 1, 0, "<None>", true);
				_RecordsetPtr rs = CreateParamRecordset("SELECT Name FROM CustomFieldsT WHERE ID = {INT}", nHealthNumberCustomField);
				if(!rs->eof) {
					CString str = AdoFldString(rs, "Name", "");
					str.TrimLeft();
					str.TrimRight();
					if(!str.IsEmpty()) {
						strOHIPHealthTitle = str;
					}
				}
				rs->Close();
			}

			//Need to hide this for blocked patients, and only show when UseOHIP() is enabled
			// (j.jones 2010-11-08 13:46) - PLID 39620 - or UseAlbertaHLINK
			if ((UseOHIP() || UseAlbertaHLINK()) && GetRemotePropertyInt("PtTB_OHIP_Health_Card", 8, 0, GetCurrentUserName(), true) > 0)
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("CASE WHEN BlockedPatientsQ.PatientID Is Null THEN OHIPHealthCardNum ELSE Null END"), _T(_bstr_t(strOHIPHealthTitle)), 18, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
			else
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("CASE WHEN BlockedPatientsQ.PatientID Is Null THEN OHIPHealthCardNum ELSE Null END"), _T(_bstr_t(strOHIPHealthTitle)), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		}
		// (j.gruber 2010-10-04 14:29) - PLID 40415
		else if (i == nSecurtyGroupCol) {
			if (GetRemotePropertyInt("PtTB_Security_Group", -9, 0, GetCurrentUserName(), true) > 0)
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("SecGroupName"), _T("Security Groups"), 18, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
			else
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(m_toolBarCombo->GetColumnCount(), _T("SecGroupName"), _T("Security Groups"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
		}
	}

	// (j.jones 2013-02-15 10:32) - PLID 40804 - remember the user's last sorted order
	short iSortColumn = (short)(GetRemotePropertyInt("PatientToolbarSortColumn", -1, 0, GetCurrentUserName(), true));
	bool bSortAsc = (GetRemotePropertyInt("PatientToolbarSortAsc", -1, 0, GetCurrentUserName(), true) == 1);
	bool bApplyDefaultSort = true;
	if(iSortColumn >= 0 && (iSortColumn + 1) <= m_toolBarCombo->GetColumnCount()) {
		//the column index is valid, is the column even displayed?
		if(m_toolBarCombo->GetColumn(iSortColumn)->GetStoredWidth() > 0) {
			//it is displayed, so this is indeed a valid, visible column
			bApplyDefaultSort = false;
		}
	}

	//if bApplyDefaultSort is true, we have no valid last user sort, so sort by patient name
	if(bApplyDefaultSort) {
		m_toolBarCombo->GetColumn(GetLastNameColumn())->PutSortPriority(0);
		m_toolBarCombo->GetColumn(GetLastNameColumn())->PutSortAscending(TRUE);
		m_toolBarCombo->GetColumn(GetFirstNameColumn())->PutSortPriority(1);
		m_toolBarCombo->GetColumn(GetFirstNameColumn())->PutSortAscending(TRUE);
		m_toolBarCombo->GetColumn(GetMiddleNameColumn())->PutSortPriority(2);
		m_toolBarCombo->GetColumn(GetMiddleNameColumn())->PutSortAscending(TRUE);
		m_toolBarCombo->GetColumn(GetPatientIDColumn())->PutSortPriority(3);
		m_toolBarCombo->GetColumn(GetPatientIDColumn())->PutSortAscending(TRUE);
	}
	else {
		//apply the user sort
		m_toolBarCombo->GetColumn(iSortColumn)->PutSortPriority(0);

		//If sorting by last name, and PatientToolbarSortLastFirstAtOnce is enabled, ApplySortLastFirstAtOnce actually reverses our sort,
		//becuase it's typically called in OnColumnClicking.
		//If it's enabled we need to initialize the sorting to be the opposite value.
		short iLastNameCol = GetLastNameColumn();
		if(iSortColumn == iLastNameCol && GetRemotePropertyInt("PatientToolbarSortLastFirstAtOnce", 1, 0, GetCurrentUserName(), true) == 1) {
			bSortAsc = !bSortAsc;
		}
		m_toolBarCombo->GetColumn(iSortColumn)->PutSortAscending(bSortAsc ? VARIANT_TRUE : VARIANT_FALSE);

		//if the sort was on last name, call ApplySortLastFirstAtOnce to check and apply
		//the preference to sort the first name as well (typically this is on)
		if(iSortColumn == iLastNameCol) {
			ApplySortLastFirstAtOnce(iSortColumn);
		}
	}

	// (a.walling 2010-05-26 16:00) - PLID 38906 - We can save a lot of memory by calculating the 'full name' in the datalist's display column
	_bstr_t bstrOriginalDisplayColumn = m_toolBarCombo->GetDisplayColumn();
	CString strDisplayColumn;
	strDisplayColumn.Format("[%hi], [%hi] [%hi]", GetLastNameColumn(), GetFirstNameColumn(), GetMiddleNameColumn());
	if(GetRemotePropertyInt("PatientToolbarShowID", 1, 0, "<None>", true) == 1) {
		strDisplayColumn += FormatString(" ([%hi])", GetPatientIDColumn());
	}
	m_toolBarCombo->PutDisplayColumn((LPCTSTR)strDisplayColumn);
}

BOOL CPatientToolBar::CreateComboBox()
{
	// (a.walling 2008-08-20 15:44) - PLID 29642 - Had to change this function a bit now that our 
	// unused buttons have actually been removed rather than ignored and worked around.
	// (a.walling 2008-08-20 15:46) - PLID 29642 - All I had to do here was use our enum rather than the define
	CRect rect, comboRect;
	int rectHeight;
	SetButtonInfo(tbCombo, IDW_COMBO, TBBS_SEPARATOR, 206);
	GetItemRect(tbCombo, &rect);
	rectHeight = rect.bottom - rect.top;
	rect.bottom = rect.top + 19;

	m_ActivePatientID = 0;

	// (c.haag 2006-04-13 15:44) - PLID 20131 - Load all patient toolbar properties into the
	// NxPropManager cache
	// (z.manning 2008-11-12 09:58) - PLID 31129 - Added PatientToolbarShowID
	// (z.manning 2008-11-12 10:16) - PLID 31129 - I added some other remote properties that were already
	// used here but not previously bulk cached.
	// (c.haag 2010-01-13 09:42) - PLID 20973 - Added PatientToolbarSortLastFirstAtOnce
	// (j.jones 2010-05-04 10:52) - PLID 32325 - added caching of OHIP settings
	// (a.walling 2010-05-21 10:12) - PLID 17768 - cache breadcrumb settings
	// (j.jones 2010-11-03 15:08) - PLID 39620 - added Alberta option
	g_propManager.CachePropertiesInBulk("PATIENTTOOLBAR", propNumber,
		"Name LIKE 'PtTB%%' OR Name IN ('PatientToolbarShowID', 'CurrentPatient', 'PatientBreadcrumbTrailMax', 'PatientBreadcrumbTrailPersist', "
		"	'DefaultPatientModuleSearch', 'DefaultPatientModuleSearch', 'PatientToolbarSortLastFirstAtOnce', "
		"	'UseOHIP', 'OHIP_HealthNumberCustomField', 'UseAlbertaHLINK', 'Alberta_PatientULICustomField', "
		"	'PatientToolbarSortColumn', " // (j.jones 2013-02-15 10:47) - PLID 40804
		"	'PatientToolbarSortAsc' "
		") "
		"	AND Username IN ('%s', '<None>')",
		_Q(GetCurrentUserName()));
	
	SetHeight(TOOLBARHEIGHT);
	rect.top -= 1;
	rect.bottom += 8;
	if (m_wndPatientsCombo.CreateControl(_T("NXDATALIST2.NxDataListCtrl.1"), "Patients Bar", WS_CHILD, rect, this, IDW_COMBO)) {

		m_toolBarCombo = m_wndPatientsCombo.GetControlUnknown();
		ASSERT(m_toolBarCombo != NULL); // We need to be able to get he control unknown and set an NxDataListPtr to point to it
		if (m_toolBarCombo) {
			try {
				// Set up the columns
				CString strDisplayColumn;
				PopulateColumns();
				//m.hancock - 5/8/2006 - PLID 20462 - Added company name to the query
				//TES 1/18/2010 - PLID 36895 - Moved the FROM clause into its own function
				m_toolBarCombo->FromClause = _bstr_t(GetFromClause());
				m_toolBarCombo->WhereClause = _bstr_t(GetDefaultFilter());
				// We want it to be a combo, not a datalist
				m_toolBarCombo->IsComboBox = TRUE;
				// (a.walling 2010-05-26 16:01) - PLID 38906 - No more FullName column -- calculate via DisplayColumn instead
				/*
				strDisplayColumn.Format("[%d]", ptbcFullName);
				m_toolBarCombo->DisplayColumn = _bstr_t(strDisplayColumn);
				m_toolBarCombo->TextSearchCol = 1;
				*/
				//m.hancock - 5/8/2006 - PLID 20462 - Changed DropDownWidth from 400 to 460
				m_toolBarCombo->DropDownWidth = 460;
				m_toolBarCombo->AutoDropDown = TRUE;
				// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
				// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - GetRemoteDataOrDataSnapshot -> GetRemoteDataSnapshot
				m_toolBarCombo->AdoConnection = GetRemoteDataSnapshot();
				m_TempLastPatientID = -1;
				m_LastPatientID = -1;
				
				m_ActivePatientID = GetRemotePropertyInt("CurrentPatient", 0, 0, GetCurrentUserName(), true);//m_toolBarCombo->GetValue(0,0).lVal);

				// (a.walling 2010-05-21 10:12) - PLID 17768 - Load saved breadcrumbs
				if (GetRemotePropertyInt("PatientBreadcrumbTrailPersist", TRUE, 0, GetCurrentUserName(), true)) {
					m_BreadcrumbTrail.Load();
				}


				//JJ - 3/21/01 - this code is similar to what is in SafeGetPatientID but doesn't pay any attention to the filters.
				//we might be able to use it soon though

				_RecordsetPtr rs;
				//if the patient that you were last on no longer exists, then load the first patient
				//also, we don't want to set the current patient to be inactive because the filter says otherwise
				// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
				// (a.walling 2013-12-12 16:51) - PLID 59996 - Snapshot isolation in Patient Tool Bar
				if(!ReturnsRecordsParam(GetRemoteDataSnapshot(), "SELECT ID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE PatientsT.PersonID = {INT} AND Archived = 0", m_ActivePatientID))
					m_ActivePatientID = 0;

				//TES 1/11/2010 - PLID 36761 - Make sure we have permission for this patient still.
				if(!GetMainFrame()->CanAccessPatient(m_ActivePatientID, true)) {
					m_ActivePatientID = 0;
				}

				//if we have no patient, get the first one in the combo
				//JJ - I feel that this is safer than doing a putcursel(0) and then a getvalue
				if(m_ActivePatientID<=0) {
					//TES 1/11/2010 - PLID 36761 - We want to pull the first patient which is not in any of the groups for which this user
					// is blocked.
					CArray<IdName,IdName> arBlockedGroups;
					GetMainFrame()->GetBlockedGroups(arBlockedGroups);
					CString strIDs;
					for(int i = 0; i < arBlockedGroups.GetSize(); i++) {
						strIDs += FormatString("%li,", arBlockedGroups[i].nID);
					}
					CString strWhere;
					if(!strIDs.IsEmpty()) {
						strIDs.TrimRight(",");
						strWhere.Format("AND ID NOT IN (SELECT PatientID FROM SecurityGroupDetailsT WHERE SecurityGroupID IN (%s))",
							strIDs);
					}
					// (a.walling 2013-12-12 16:51) - PLID 59996 - Snapshot isolation in Patient Tool Bar
					rs = CreateRecordset(GetRemoteDataSnapshot(), "SELECT TOP 1 ID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE CurrentStatus <> 4 AND ID > 0 %s ORDER BY Last ASC, First ASC", strWhere);
					if(!rs->eof) {
						m_ActivePatientID = rs->Fields->Item["ID"]->Value.lVal;
					}
				}
				m_toolBarCombo->Requery();
				long nTrySetSelResult = m_toolBarCombo->TrySetSelByColumn_Deprecated(0,(long)m_ActivePatientID);
				// (a.walling 2010-06-29 07:51) - PLID 17768 - TrySetSel may return immediately without firing an event; ensure we handle any changes.
				if (nTrySetSelResult >= 0) {
					DropBreadcrumb(m_ActivePatientID);
				}
			} NxCatchAll("Error in creating patient bar!");
			// We have success so display the window
			m_wndPatientsCombo.ShowWindow(SW_SHOW);
			// Done, return success
		
			// center combo box edit control vertically within tool bar
			m_wndPatientsCombo.GetWindowRect(&comboRect);
			m_wndPatientsCombo.ScreenToClient(&comboRect);  
			m_wndPatientsCombo.SetWindowPos(&wndTopMost, rect.left, 
				rect.top + (rectHeight - comboRect.Height())/2+1, 0, 0, 
				SWP_NOSIZE|SWP_NOACTIVATE);
				return TRUE;
			}
			else {
				// Failed because we couldn't set dlp to the control unknown
				MsgBox("Could not connect to the patients toolbar.");
				m_wndPatientsCombo.DestroyWindow();
				return FALSE;
			}
	} else {
		// Couldn't create the control in the first place.  Is NxDataList registered on this computer?
		DWORD dwError = GetLastError();
		CString str;
		str.Format("GetLastError returned %li when CreateControl failed.", dwError);
		MsgBox(str);
		return FALSE;
	}
	return TRUE;
}

// (a.walling 2010-05-21 10:12) - PLID 17768 - Let the toolbar save the last patient (along with the other patient history info)
void CPatientToolBar::SaveLastPatient()
{
	try {
		SetRemotePropertyInt("CurrentPatient", GetActivePatientID(), 0, GetCurrentUserName());
		if (GetRemotePropertyInt("PatientBreadcrumbTrailPersist", TRUE, 0, GetCurrentUserName(), true)) {
			// (a.walling 2010-05-21 10:12) - PLID 17768 - Save breadcrumbs
			m_BreadcrumbTrail.Save();
		}
	} catch (...) {
		ASSERT(FALSE);
	};
}

void CPatientToolBar::OnSelChosenPatientCombo(LPDISPATCH lpRow)
{
	// (j.jones 2012-08-08 09:28) - PLID 51063 - added a try/catch
	try {

		CMainFrame *pMainFrame = GetMainFrame();

		IRowSettingsPtr pNewRow(lpRow);

		if(!pNewRow) {
			//don't allow unselected
			pNewRow = FindRowByPersonID(m_ActivePatientID);
			m_toolBarCombo->CurSel = pNewRow;
			m_toolBarCombo->EnsureRowInView(pNewRow);
		}

		//TES 1/11/2010 - PLID 36761 - Make sure we have permission for this patient.
		long nNewPatientID = VarLong(pNewRow->GetValue(0));
		if(!GetMainFrame()->CanAccessPatient(nNewPatientID, false)) {			
			pNewRow = FindRowByPersonID(m_ActivePatientID);
			m_toolBarCombo->CurSel = pNewRow;
			m_toolBarCombo->EnsureRowInView(pNewRow);
		}

		m_LastPatientID = m_ActivePatientID;
		m_ActivePatientID = pNewRow->GetValue(0).lVal;
		DropBreadcrumb(m_ActivePatientID);

		if (pMainFrame)
		{	pMainFrame->SetStatusButtons();
			if (pMainFrame->GetActiveView())
				pMainFrame->GetActiveView()->UpdateView();

			pMainFrame->HandleActivePatientChange(); // (z.manning 2009-05-19 12:38) - PLID 28512
		}

	}NxCatchAll(__FUNCTION__);
}

void CPatientToolBar::OnRequeryFinishedPatientCombo(short nFlags) 
{
	try {
		//if the requery successfully finished
		if(nFlags==0) {
			//ensure that something was selected
			if(!m_toolBarCombo->GetCurSel()) {
				//set the current selection to the first patient
				m_toolBarCombo->CurSel = m_toolBarCombo->GetFirstRow();

				if(m_toolBarCombo->GetCurSel()) {
					//TES 1/11/2010 - PLID 36761 - Call SafeGet in case we don't have access to this patient.
					m_ActivePatientID = SafeGetPatientID(VarLong(m_toolBarCombo->GetCurSel()->GetValue(0)));
					//TES 1/18/2010 - PLID 36761 - Need to make sure we reflect the new selection in the list.					
					long nTrySetSelResult = m_toolBarCombo->TrySetSelByColumn_Deprecated(0,(long)m_ActivePatientID);
					// (a.walling 2010-06-29 07:51) - PLID 17768 - TrySetSel may return immediately without firing an event; ensure we handle any changes.
					if (nTrySetSelResult >= 0) {
						DropBreadcrumb(m_ActivePatientID);
					}
					if(GetMainFrame() && GetMainFrame()->GetActiveView())
						GetMainFrame()->GetActiveView()->UpdateView();

					GetMainFrame()->HandleActivePatientChange(); // (z.manning 2009-05-19 12:38) - PLID 28512
				}
			}

			//JJ - 1/23/01 - try to not set the patient count until the requery is done
			CString str;
			long nCurSelIndex = 0;
			IRowSettingsPtr pCurSelRow = m_toolBarCombo->GetCurSel();
			if (pCurSelRow) {
				nCurSelIndex = pCurSelRow->CalcRowNumber() + 1;
			}
			str.Format("%i / %i", nCurSelIndex, m_toolBarCombo->GetRowCount());
			m_text3.SetWindowText(str);


			//exit if something was set
			if(m_toolBarCombo->GetCurSel())
				return;

			//if this assertion is called, no selection was set after doing a TrySetSelByColumn() in the 
			//CreateComboBox() function
			ASSERT(false);
		}
	} NxCatchAll("Error in OnRequeryFinishedPatientCombo");
}

// Throws _com_error
// Returns the active patient's ID or 0 if no active patient (only because it used to, I really 
// think it should return -1 but I can't be sure how it's being used elsewhere - RAC)
long CPatientToolBar::GetActivePatientID()
{
	return m_ActivePatientID;
	/* JJ - we don't need to query the bar anymore
	if (m_toolBarCombo) {
		// The toolbar exists
		long nCurSel = m_toolBarCombo->CurSel;
		if (nCurSel >= 0) {
			// There is a current selection so return the ID
			return VarLong(m_toolBarCombo->Value[nCurSel][0]);
		}
	}

	// Returns 0 if no active patient (only because it used to, I really think it 
	// should return -1 but I can't be sure how it's being used elsewhere - RAC)
	return 0; */
}

CString CPatientToolBar::GetActivePatientName()
{
	if (m_toolBarCombo) {
		// The toolbar exists
		IRowSettingsPtr pCurSel = m_toolBarCombo->CurSel;
		if (pCurSel) {
			// There is a current selection so return the name
			// (z.manning 2008-11-17 10:37) - PLID 31129 - Use the GetFullName function.
			return GetFullName();
		}
	}
	
	return "";
}

// (j.jones 2008-10-31 13:20) - PLID 31891 - supported an optional passed-in connection
// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
CString CPatientToolBar::GetExistingPatientName(long PatientID)
{
	if (PatientID == -1) {
		return "";
	}
	//this will search the datalist for the patient name, which should be faster
	//than querying a recordset

	if (IsMainThread() && GetMainFrame()) {
		if(m_toolBarCombo) {
			// The toolbar exists
			IRowSettingsPtr pCurSel = FindRowByPersonID(PatientID, false);
			if (pCurSel) {
				// The patient exisits so return the name
				// (z.manning 2008-11-17 10:57) - PLID 31129 - Use the GetFullNameByRow function
				return GetFullNameByRow(pCurSel);
			}
		}
	}

	//fine, query the recordset
	// (a.walling 2013-12-12 16:51) - PLID 59996 - Snapshot isolation in Patient Tool Bar
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT [Last] + ', ' + [First] + ' ' + [Middle] AS FullName FROM PersonT WHERE ID = {INT}",PatientID);
	if(!rs->eof) {
		return AdoFldString(rs, "FullName","");
	}

	return "";
}

// (b.savon 2011-11-16 15:28) - PLID 45433 - Thread safe, Does Patient Exist?  For Device Import
// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
BOOL CPatientToolBar::DoesPatientExistByUserDefinedID(const long nUserDefinedID)
{
	if( IsMainThread() && GetMainFrame() ){
		// (a.walling 2015-02-19 08:56) - PLID 64653 - Do not FindByColumn if requerying since it will block until column is found or requery is done
		if( m_toolBarCombo && !m_toolBarCombo->IsRequerying() ){
			IRowSettingsPtr pCurSel = m_toolBarCombo->FindByColumn(GetPatientIDColumn(), (long)nUserDefinedID, 0, FALSE);
			if( pCurSel ){
				return TRUE;
			}
		}
	}

	// (a.walling 2013-12-12 16:51) - PLID 59996 - Snapshot isolation in Patient Tool Bar
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT UserDefinedID FROM PersonT INNER JOIN PatientsT ON PersonT.ID = PatientsT.PersonID WHERE UserDefinedID = {INT}",nUserDefinedID);
	if(!rs->eof) {
		return AdoFldLong(rs, "UserDefinedID", -1) == -1 ? FALSE : TRUE;
	}

	return FALSE;

}

// (j.jones 2007-07-17 09:21) - PLID 26702 - added birthdate functions
COleDateTime CPatientToolBar::GetActivePatientBirthDate()
{
	short nBirthDateColumn = GetBirthDateColumn();

	COleDateTime dtBirthDate;
	dtBirthDate.SetStatus(COleDateTime::invalid);

	if (m_toolBarCombo) {
		// The toolbar exists
		IRowSettingsPtr pCurSel = m_toolBarCombo->CurSel;
		if (pCurSel) {
			// There is a current selection so return the birthdate
			return GetPatientBirthDateByRow(pCurSel);
		}
	}
	
	return dtBirthDate;
}

// (j.jones 2007-07-17 09:21) - PLID 26702 - added birthdate functions
COleDateTime CPatientToolBar::GetExistingPatientBirthDate(long PatientID)
{
	COleDateTime dtBirthDate;
	dtBirthDate.SetStatus(COleDateTime::invalid);

	//this will search the datalist for the patient birthdate, which should be faster
	//than querying a recordset

	// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
	if (IsMainThread() && GetMainFrame()) {
		if(m_toolBarCombo) {
			// The toolbar exists
			IRowSettingsPtr pCurSel = FindRowByPersonID(PatientID, false);
			if (pCurSel) {
				// There is a current selection so return the birthdate
				return GetPatientBirthDateByRow(pCurSel);
			}
		}
	}
	
	//fine, query the recordset
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	// (a.walling 2013-12-12 16:51) - PLID 59996 - Snapshot isolation in Patient Tool Bar
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT BirthDate FROM PersonT WHERE ID = {INT}",PatientID);
	if(!rs->eof) {
		return AdoFldDateTime(rs, "BirthDate",dtBirthDate);
	}

	return dtBirthDate;
}

// (a.walling 2010-05-21 10:12) - PLID 17768 - Helper functions
COleDateTime CPatientToolBar::GetPatientBirthDateByRow(IRowSettingsPtr pRow)
{
	COleDateTime dtBirthDate;
	dtBirthDate.SetStatus(COleDateTime::invalid);

	return VarDateTime(pRow->Value[GetBirthDateColumn()], dtBirthDate);
}

// (z.manning 2009-05-05 10:44) - PLID 28529 - Added user defined ID functions
long CPatientToolBar::GetActivePatientUserDefinedID()
{
	if (m_toolBarCombo) {
		// The toolbar exists
		IRowSettingsPtr pCurSel = m_toolBarCombo->CurSel;
		if (pCurSel) {
			// There is a current selection so return the user defined id
			return GetUserDefinedIDByRow(pCurSel);
		}
	}

	// (z.manning 2015-03-30 15:53) - NX-100431 - If that didn't work, try getting it from data
	long nActivePatientID = GetActivePatientID();
	if (nActivePatientID > 0)
	{
		_RecordsetPtr prsID = CreateParamRecordset(
			"SELECT Pat.UserDefinedID FROM PatientsT Pat WHERE Pat.PersonID = {INT}"
			, nActivePatientID);
		if (!prsID->eof) {
			return AdoFldLong(prsID, "UserDefinedID");
		}
	}
	
	return -1;
}

// (z.manning 2009-05-05 10:44) - PLID 28529 - Added user defined ID functions
// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
long CPatientToolBar::GetExistingPatientUserDefinedID(const long nPatientID)
{
	//this will search the datalist for the patient user defined id, which should be faster
	//than querying a recordset

	if (IsMainThread() && GetMainFrame()) {
		if(m_toolBarCombo) {
			// The toolbar exists
			IRowSettingsPtr pCurSel = FindRowByPersonID(nPatientID, false);
			if (pCurSel) {
				// There is a current selection so return the user defined id
				return GetUserDefinedIDByRow(pCurSel);
			}
		}
	}
	
	//fine, query the recordset
	// (a.walling 2013-12-12 16:51) - PLID 59996 - Snapshot isolation in Patient Tool Bar
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT UserDefinedID FROM PatientsT WHERE PersonID = {INT}", nPatientID);

	if(!rs->eof) {
		return AdoFldLong(rs->GetFields(), "UserDefinedID", -1);
	}

	return -1;
}

// (a.wilson 2013-01-10 10:26) - PLID 54515 - created to get patient internal id based on patient userdefined id.
// (a.walling 2014-01-06 12:43) - PLID 59996 - No connection pointers for these existing patient functions
long CPatientToolBar::GetExistingPatientIDByUserDefinedID(long nPatientUserDefinedID)
{
	if (IsMainThread() && GetMainFrame()) {
		// (a.walling 2015-02-19 08:56) - PLID 64653 - Do not FindByColumn if requerying since it will block until column is found or requery is done
		if(m_toolBarCombo && !m_toolBarCombo->IsRequerying() ) {
			// The toolbar exists
			IRowSettingsPtr pCurSel = m_toolBarCombo->FindByColumn(GetPatientIDColumn(), nPatientUserDefinedID, 0, FALSE);
			if (pCurSel) {
				// There is a current selection so return the patient id
				return VarLong(pCurSel->GetValue(0), -1);
			}
		}
	}
	//no mainframe? no pattoolbar? fine, query the recordset

	// (a.walling 2013-12-12 16:51) - PLID 59996 - Snapshot isolation in Patient Tool Bar
	_RecordsetPtr rs = CreateParamRecordset(GetRemoteDataSnapshot(), "SELECT PersonID FROM PatientsT WHERE UserDefinedID = {INT}", nPatientUserDefinedID);

	if (!rs->eof) {
		return AdoFldLong(rs->GetFields(), "PersonID", -1);
	}

	return -1;
}

// (a.walling 2010-05-21 10:12) - PLID 17768 - Helper functions
long CPatientToolBar::GetUserDefinedIDByRow(IRowSettingsPtr pRow)
{
	return VarLong(pRow->Value[GetPatientIDColumn()], -1);
}

//TES 1/6/2010 - PLID 36761 - This now may fail, if the current user doesn't have permission to view this patient.  In that
// case, this function will give an explanatory message and return FALSE.  Otherwise, it will return TRUE.
BOOL CPatientToolBar::TrySetActivePatientID(long newPatientID)
{
	if(newPatientID <= 0)
		return FALSE;

	//if it already is the active patient ID, do nothing!
	if(newPatientID == m_ActivePatientID)
		return TRUE;

	//TES 1/11/2010 - PLID 36761 - Make sure we have permission for this patient.
	if(!GetMainFrame()->CanAccessPatient(newPatientID, false)) {
		return FALSE;
	}

	m_LastPatientID = m_ActivePatientID;
	m_ActivePatientID = newPatientID;
	CString str,strSQL;
	strSQL = GetMainFrame()->GetPatientSQL();
	if(strSQL!="")
		strSQL = "AND " + strSQL;
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	str.Format("SELECT ID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE PersonID = {INT} %s",strSQL);

	BOOL bRequery = FALSE;

	// (z.manning 2008-12-10 15:18) - PLID 32397 - Fixed IsRecordsetEmpty call to prevent text formatting errors
	if(ReturnsRecordsParam(str, newPatientID)) {
		long nRow = m_toolBarCombo->TrySetSelByColumn_Deprecated(0,newPatientID);
		// (a.walling 2010-06-29 07:51) - PLID 17768 - TrySetSel may return immediately without firing an event; ensure we handle any changes.
		if (nRow >= 0) {
			DropBreadcrumb(newPatientID);
		}
		if(nRow == -1)
			bRequery = TRUE;
	}
	else
		bRequery = TRUE;
	
	if(bRequery) {
		m_butAll.SetCheck(true);
		m_butActive.SetCheck(false);
		m_butFilter.SetCheck(false);
		m_butAllSearchCheck.SetCheck(true);
		m_butPatientSearchCheck.SetCheck(false);
		m_butProspectSearchCheck.SetCheck(false);
		SetComboSQL("",TRUE,FALSE,newPatientID);
	}
	UpdateStatusText();

	//TES 9/21/2012 - PLID 52565 - Fixes a problem where if the recent patient list obscured the tool bar, that section of the toolbar 
	// wouldn't redraw itself.
	Invalidate();
	GetMainFrame()->HandleActivePatientChange(); // (z.manning 2009-05-19 12:38) - PLID 28512

	// (a.walling 2010-08-18 11:46) - PLID 17768 - This is handled elsewhere
	//DropBreadcrumb(newPatientID);

	return TRUE;
}

// (a.walling 2007-05-04 09:53) - PLID 4850 - Called when the user has been changed, refresh any user-specific settings
void CPatientToolBar::OnUserChanged()
{
	try {
		long nDefaultPatientModuleSearch = GetRemotePropertyInt("DefaultPatientModuleSearch",0,0,GetCurrentUserName(),TRUE);
		long nDefaultPatientModuleInclude = GetRemotePropertyInt("DefaultPatientModuleInclude",1,0,GetCurrentUserName(),TRUE);

		m_butPatientSearchCheck.SetCheck(nDefaultPatientModuleSearch == 1);
		m_butActive.SetCheck(nDefaultPatientModuleInclude==1);
		m_butProspectSearchCheck.SetCheck(nDefaultPatientModuleSearch==2);
		m_butAll.SetCheck(nDefaultPatientModuleInclude==0);
		m_butAllSearchCheck.SetCheck(nDefaultPatientModuleSearch==0);

		m_butFilter.SetCheck(FALSE);
		
		PopulateColumns();

		if(GetMainFrame() != NULL) {
			// (z.manning 2009-06-15 12:18) - PLID 34614 - We need to regenerate the filters in case
			// the previous user had a lookup active.
			GetMainFrame()->GenerateFilters();
		}

		// (a.walling 2009-06-09 09:56) - PLID 34390 - Since the columns may have been flipped around, we need to requery the list.
		// otherwise functions like getexistingpatientname may end up throwing bad variable type errors.
		Requery();

		// (a.walling 2010-05-21 10:12) - PLID 17768 - Reload the breadcrumb list
		m_BreadcrumbTrail.Clear();
		
		if (GetRemotePropertyInt("PatientBreadcrumbTrailPersist", TRUE, 0, GetCurrentUserName(), true)) {
			m_BreadcrumbTrail.Load();
		}

		// (j.jones 2007-08-09 16:35) - PLID 27036 - added ability to keep the current patient/contact when switching users
		long nSwitchUser_KeepCurrentPatientContact = GetRemotePropertyInt("SwitchUser_KeepCurrentPatientContact", 0, 0, GetCurrentUserName(), true);
		//0 - keep the current patient, 1 - load the new user's last patient
		if(nSwitchUser_KeepCurrentPatientContact == 1) {
			//TES 1/7/2010 - PLID 36761 - This function may fail now
			TrySetActivePatientID(GetRemotePropertyInt("CurrentPatient", 0, 0, GetCurrentUserName(), true));
		}
	} NxCatchAllThrow("Error initializing patient toolbar");
}

// (a.walling 2010-08-19 08:17) - PLID 17768 - Called when preferences have been changed
void CPatientToolBar::OnPreferencesChanged()
{
	try {
		m_BreadcrumbTrail.Prune(); // if the limit was decreased, this should prune it.
	} NxCatchAll(__FUNCTION__);
}

void CPatientToolBar::Requery()
{
	TRACE("CPatientToolBar::Requery <==> Requerying Patients Combo!\n");
	m_rowCache.clear();
	m_text3.SetWindowText("Loading...");
	//TES 1/18/2010 - PLID 36895 - Make sure the FROM clause is up to date.
	m_toolBarCombo->FromClause = _bstr_t(GetFromClause());
	m_toolBarCombo->Requery();	
	long nTrySetSelResult = m_toolBarCombo->TrySetSelByColumn_Deprecated(0,(long)m_ActivePatientID);
	// (a.walling 2010-06-29 07:51) - PLID 17768 - TrySetSel may return immediately without firing an event; ensure we handle any changes.
	if (nTrySetSelResult >= 0) {
		DropBreadcrumb(m_ActivePatientID);
	}
}

long CPatientToolBar::IsActiveChecked()
{
	return m_butActive.GetCheck();
}

BOOL CPatientToolBar::CreateSearchButtons()
{
	// (a.walling 2008-08-20 15:44) - PLID 29642 - Had to change this function a bit now that our 
	// unused buttons have actually been removed rather than ignored and worked around.

	// (a.walling 2008-08-20 15:45) - PLID 29642 - Basically, all the search controls are now on
	// one separator button.
	CRect rect, trueRect;
//	int rectHeight;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	// (a.walling 2008-11-13 10:46) - PLID 31956 - Now that NxButton is fixed, we can revert back to DEFAULT_QUALITY
	// (a.walling 2016-06-01 11:12) - NX-100195 - use Segoe UI
	CreateCompatiblePointFont(&m_searchButtonFont, 100, "Segoe UI", NULL, DEFAULT_QUALITY);
	const int	tbWidth1 = 65,//65,
				tbWidth2 = 70,//75,
				tbWidth3 = 80,//88,
				tbWidth4 = 50;//40;
 
	SetButtonInfo(tbSearch, ID_BUTTON32861, TBBS_SEPARATOR, tbWidth1 + tbWidth2 + tbWidth3 + tbWidth4);
	GetItemRect(tbSearch, &trueRect);
	rect = trueRect;
	rect.bottom = rect.top + 15;
	rect.top -= 1;
	rect.right = rect.left + tbWidth1; // (a.walling 2008-08-20 08:43) - PLID 
	if(m_text.Create("  Search:", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT, rect, this, IDC_TEXT1_TOOLBAR) == 0)
		return FALSE;
	m_text.SetFont (&m_searchButtonFont, FALSE);

	rect = trueRect;
	rect.top += 15;
	rect.top -= 1;
	rect.bottom = 	rect.top + 15;
	rect.right = rect.left + tbWidth1; // (a.walling 2008-08-20 08:43) - PLID 
	if(m_text2.Create("  Include:", WS_CHILD|WS_VISIBLE|SS_LEFT, rect, this, IDC_TEXT2_TOOLBAR) == 0)
		return FALSE;
	m_text2.SetFont (&m_searchButtonFont, FALSE);

	long nDefaultPatientModuleSearch = GetRemotePropertyInt("DefaultPatientModuleSearch",0,0,GetCurrentUserName(),TRUE);

	//SetButtonInfo(BUTTON2, IDW_COMBO, TBBS_SEPARATOR, width2);
	//GetItemRect(BUTTON2, &trueRect);
	rect = trueRect;
	rect.left += tbWidth1; // (a.walling 2008-08-20 08:43) - PLID 
	rect.right = rect.left + (tbWidth2); // (a.walling 2008-08-20 08:44) - PLID 
	rect.bottom = rect.top + 15;
	if(m_butPatientSearchCheck.Create("Patients", WS_TABSTOP|WS_VISIBLE|BS_AUTORADIOBUTTON, rect, this, IDC_PATIENTS_SEARCH) == 0)
		return FALSE;
	m_butPatientSearchCheck.SetCheck(nDefaultPatientModuleSearch == 1);
	m_butPatientSearchCheck.SetFont (&m_searchButtonFont, FALSE);

	long nDefaultPatientModuleInclude = GetRemotePropertyInt("DefaultPatientModuleInclude",1,0,GetCurrentUserName(),TRUE);

	rect		= trueRect;
	rect.left += tbWidth1; // (a.walling 2008-08-20 08:43) - PLID 
	rect.right = rect.left + (tbWidth2); // (a.walling 2008-08-20 08:44) - PLID 
	rect.top	+= 15;
	rect.bottom = 	rect.top + 15;
	if(m_butActive.Create("Active", WS_TABSTOP|WS_VISIBLE|BS_AUTORADIOBUTTON, rect, this, IDC_ACTIVE_TOOLBAR) == 0)
		return FALSE;
	m_butActive.SetCheck(nDefaultPatientModuleInclude==1);
	m_butActive.SetFont (&m_searchButtonFont, FALSE);

	//SetButtonInfo(BUTTON3, IDW_COMBO, TBBS_SEPARATOR, width3);
	//GetItemRect(BUTTON3, &trueRect);
	rect = trueRect;
	rect.left += (tbWidth1 + tbWidth2); // (a.walling 2008-08-20 08:43) - PLID 
	rect.right = rect.left + (tbWidth3); // (a.walling 2008-08-20 08:44) - PLID 
	rect.bottom = rect.top + 15;
	if(m_butProspectSearchCheck.Create("Prospects", WS_TABSTOP|WS_VISIBLE|BS_AUTORADIOBUTTON, rect, this, IDC_PROSPECT_SEARCH) == 0)
		return FALSE;
	m_butProspectSearchCheck.SetCheck(nDefaultPatientModuleSearch==2);
	m_butProspectSearchCheck.SetFont (&m_searchButtonFont, FALSE);

	rect = trueRect;
	rect.left += (tbWidth1 + tbWidth2); // (a.walling 2008-08-20 08:43) - PLID 
	rect.right = rect.left + (tbWidth3); // (a.walling 2008-08-20 08:44) - PLID 
	rect.top += 15;
	rect.bottom = 	rect.top + 15;
	rect.right -= 30;
	if(m_butAll.Create("All", WS_TABSTOP|WS_VISIBLE|BS_AUTORADIOBUTTON, rect, this, IDC_ALL_TOOLBAR) == 0)
		return FALSE;
	m_butAll.SetCheck(nDefaultPatientModuleInclude==0);
	m_butAll.SetFont (&m_searchButtonFont, FALSE);

	//SetButtonInfo(BUTTON4, IDW_COMBO, TBBS_SEPARATOR, width4);
	//GetItemRect(BUTTON4, &trueRect);
	rect = trueRect;
	rect.left += (tbWidth1 + tbWidth2 + tbWidth3); // (a.walling 2008-08-20 08:43) - PLID 
	rect.right = rect.left + (tbWidth4); // (a.walling 2008-08-20 08:44) - PLID 
	rect.bottom = rect.top + 15;
	if(m_butAllSearchCheck.Create("All", WS_TABSTOP|WS_VISIBLE|BS_AUTORADIOBUTTON, rect, this, IDC_ALL_SEARCH) == 0)
		return FALSE;
	m_butAllSearchCheck.SetCheck(nDefaultPatientModuleSearch==0);
	m_butAllSearchCheck.SetFont (&m_searchButtonFont, FALSE);

	rect = trueRect;
	rect.left += (tbWidth1 + tbWidth2 + tbWidth3); // (a.walling 2008-08-20 08:43) - PLID 
	rect.right = rect.left + (tbWidth4); // (a.walling 2008-08-20 08:44) - PLID 
	rect.top += 15;
	rect.bottom = rect.top + 15;
	rect.left -= 30;
	rect.right -= 15;
	if(m_butFilter.Create("Lookup", WS_TABSTOP|WS_VISIBLE|BS_AUTOCHECKBOX, rect, this, IDC_FILTER) == 0)
		return FALSE;
	m_butFilter.SetFont (&m_searchButtonFont, FALSE);

/*	rect = trueRect;
	rect.top += 15;
	rect.bottom = rect.top + 15;
	if(m_blank1.Create("", WS_CHILD|WS_VISIBLE|SS_LEFT, rect, this, 1871) == 0)
		return FALSE;*/

	// (a.walling 2007-11-12 13:30) - PLID 28062 - Now that everything is in place, hide the
	// buttons we no longer use, and expand Button1 to cover where they would normally be.
	/*SetButtonInfo(BUTTON1, IDW_COMBO, TBBS_SEPARATOR, width1 + width2 + width3 + width4);
	SetButtonInfo(BUTTON2, IDW_COMBO, TBBS_SEPARATOR|TBBS_HIDDEN, width2);
	SetButtonInfo(BUTTON3, IDW_COMBO, TBBS_SEPARATOR|TBBS_HIDDEN, width3);
	SetButtonInfo(BUTTON4, IDW_COMBO, TBBS_SEPARATOR|TBBS_HIDDEN, width4);*/

	
	SetButtonInfo(tbSearch, ID_BUTTON32861, TBBS_SEPARATOR|TBSTATE_HIDDEN, tbWidth1 + tbWidth2 + tbWidth3 + tbWidth4);
	return TRUE;
}

BOOL CPatientToolBar::CreateRightOfComboButtons()
{
	// (a.walling 2008-08-20 15:44) - PLID 29642 - Had to change this function a bit now that our 
	// unused buttons have actually been removed rather than ignored and worked around.

	// (a.walling 2008-08-20 15:46) - PLID 29642 - The 'empty static' controls are now gone.

	// First create an empty static text to hide the buttons
	const int nButtonWidth = 40;
	{
		// Set the buttons as separators so that they don't try to do anything fancy when the mouse is over them
		// (Who named these IDs???????????)
		// (a.walling 2007-11-12 13:29) - PLID 28062 - These separators look very annoying. So hide the buttons we don't
		// use and expand the RIGHT1 button over them.
		/*
		SetButtonInfo(RIGHT1, ID_BUTTON234243252, TBBS_SEPARATOR, nButtonWidth * 3);
		SetButtonInfo(RIGHT2, ID_BUTTON32867, TBBS_SEPARATOR|TBBS_HIDDEN, nButtonWidth);
		SetButtonInfo(RIGHT3, ID_BUTTON32868, TBBS_SEPARATOR|TBBS_HIDDEN, nButtonWidth);
		*/
		
		// Then create a static control that covers all the buttons to hide them
		/*CRect rect;
		GetItemRect(RIGHT1, rect);
		rect.right = rect.left + 3*nButtonWidth;

		// We really don't care if this works, it's purely cosmetic
		m_blank4.Create("", WS_CHILD|WS_VISIBLE|WS_GROUP|SS_CENTER, rect, this);*/
	}

	// Now create the static on which we will be placing the position text

	// (a.walling 2010-05-21 10:12) - PLID 17768 - Breadcrumbs button
	// (a.walling 2010-05-26 16:45) - PLID 38906 - We are having more offices with 6-digit counts of patients, so expand the size for the text a bit.
	SetButtonInfo(tbCount, ID_BUTTON_BREADCRUMBS, TBBS_SEPARATOR, nButtonWidth * 4);
	CRect rect;
	GetItemRect(tbCount, &rect);
	// (a.walling 2010-05-26 16:45) - PLID 38906 - Also added 2 pixels to make this aligned better
	rect.top+= 7;
	// (a.walling 2007-11-12 13:29) - PLID 28062 - No longer necessary; we expanded the RIGHT1 button already.
	// rect.right+= 70;

	CString strPos;
	strPos.Format("Loading...");
	if(m_text3.Create(strPos, WS_CHILD|WS_VISIBLE|WS_GROUP|SS_CENTER, rect, this, IDC_ACTIVE_TOOLBAR) == 0)
		return FALSE;

	m_pFont = new CFont;
	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	CreateCompatiblePointFont(m_pFont, 120, "Segoe UI");

	m_text3.SetFont (m_pFont);

	return TRUE;
}

void CPatientToolBar::OnEnable(BOOL bEnable) 
{
	CToolBar::OnEnable(bEnable);
	
	m_butActive.EnableWindow(bEnable);
	m_butAll.EnableWindow(bEnable);
	m_butPatientSearchCheck.EnableWindow(bEnable);
	m_butAllSearchCheck.EnableWindow(bEnable);
	m_butProspectSearchCheck.EnableWindow(bEnable);
	m_butPatient.EnableWindow(bEnable);
	m_butProspect.EnableWindow(bEnable);
	m_butPatientProspect.EnableWindow(bEnable);
	m_butFilter.EnableWindow(bEnable);
	m_text.EnableWindow(bEnable);
	m_text2.EnableWindow(bEnable);
	m_text3.EnableWindow(bEnable);
}

bool CPatientToolBar::SetComboSQL(const CString &strNewSQL, bool bForce /* = false */, bool bManual /*=false*/, long id /* = -25*/)
{
	long currID = 0;
	if(id != -25)
		currID = id;
	else if(m_toolBarCombo->GetCurSel())
		currID = m_toolBarCombo->GetCurSel()->GetValue(0).lVal;
	CString str, strSQL = strNewSQL;
	_bstr_t str1;
	str1 = m_toolBarCombo->WhereClause;
	str = (LPCTSTR)str1;

	strSQL.TrimRight(';');
	if (bForce || str.CompareNoCase(strSQL) != 0)
	{	TRACE("CPatientToolBar::SetComboSQL <==> Changing Patient Combo SQL Statement!\n");
		if(strNewSQL!="")
			strSQL = "WHERE " + strNewSQL;
		//TES 1/25/2010 - PLID 36761 - We need to return FALSE if there are no _accessible_ patients.
		CArray<IdName,IdName> arBlockedGroups;
		GetMainFrame()->GetBlockedGroups(arBlockedGroups);
		CString strIDs;
		for(int i = 0; i < arBlockedGroups.GetSize(); i++) {
			strIDs += FormatString("%li,", arBlockedGroups[i].nID);
		}
		CString strWhere;
		if(!strIDs.IsEmpty()) {
			strIDs.TrimRight(",");
			strWhere.Format("AND ID NOT IN (SELECT PatientID FROM SecurityGroupDetailsT WHERE SecurityGroupID IN (%s))",
				strIDs);
		}
		if(IsRecordsetEmpty("SELECT ID FROM (SELECT * FROM PatientsT WHERE PersonID > 0) AS PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID %s %s",strSQL, strWhere))
			return FALSE;
		m_toolBarCombo->WhereClause = _bstr_t(strNewSQL);
		if (!bManual)
		{
			Requery();
			long id = SafeGetPatientID(currID);
			if(id!=currID) {
				// (b.cardillo 2004-03-09 09:29) - PLID 11344 - We used to set m_TempLastPatientID to m_LastPatientID 
				// here, which resulted in hopping TWO patients backward when the user hit Ctrl-L after this filter 
				// changed.
				m_TempLastPatientID = m_ActivePatientID;
				m_LastPatientID = -1;
				m_ActivePatientID = id;
				DropBreadcrumb(m_ActivePatientID);
				if(GetMainFrame() != NULL) {
					GetMainFrame()->HandleActivePatientChange();
				}
			}
			try {				
				long nTrySetSelResult = m_toolBarCombo->TrySetSelByColumn_Deprecated(0,(long)id);
				// (a.walling 2010-06-29 07:51) - PLID 17768 - TrySetSel may return immediately without firing an event; ensure we handle any changes.
				if (nTrySetSelResult >= 0) {
					DropBreadcrumb(id);
				}
			} NxCatchAll("Error in SetComboSQL");
		}
		return true;
	} 
	else 
	{	//if (!bManual)
		//	m_toolBarCombo.SetSQLStatement(strSQL);
		return false;
	}
}

short CPatientToolBar::GetFirstNameColumn()
{
	return (short)abs(GetRemotePropertyInt("PtTB_First Name", 2, 0, GetCurrentUserName(), true)) + ptbcHardColumnCount - 1;
}

short CPatientToolBar::GetMiddleNameColumn()
{
	return (short)abs(GetRemotePropertyInt("PtTB_Middle Name", 3, 0, GetCurrentUserName(), true)) + ptbcHardColumnCount - 1;
}

short CPatientToolBar::GetLastNameColumn()
{
	return (short)abs(GetRemotePropertyInt("PtTB_Last Name", 1, 0, GetCurrentUserName(), true)) + ptbcHardColumnCount - 1;
}

short CPatientToolBar::GetPatientIDColumn()
{
	return (short)abs(GetRemotePropertyInt("PtTB_Patient ID", 4, 0, GetCurrentUserName(), true)) + ptbcHardColumnCount - 1;
}

short CPatientToolBar::GetBirthDateColumn()
{
	return (short)abs(GetRemotePropertyInt("PtTB_Birth Date", 5, 0, GetCurrentUserName(), true)) + ptbcHardColumnCount - 1;
}

short CPatientToolBar::GetSSNColumn()
{
	return (short)abs(GetRemotePropertyInt("PtTB_SSN", 6, 0, GetCurrentUserName(), true)) + ptbcHardColumnCount - 1;
}

//m.hancock - 5/8/2006 - PLID 20462 - Add the company name as an option to the new patient toolbar
short CPatientToolBar::GetCompanyColumn()
{
	return (short)abs(GetRemotePropertyInt("PtTB_Company or School", -7, 0, GetCurrentUserName(), true)) + ptbcHardColumnCount - 1;
}

// (j.jones 2010-05-04 10:55) - PLID 32325 - added OHIP Health Card Column
short CPatientToolBar::GetOHIPHealthCardColumn()
{
	return (short)abs(GetRemotePropertyInt("PtTB_OHIP_Health_Card", 8, 0, GetCurrentUserName(), true)) + ptbcHardColumnCount - 1;
}

// (j.gruber 2010-10-04 14:29) - PLID 4015 - added security group column
short CPatientToolBar::GetSecurityGroupColumn()
{
	return (short)abs(GetRemotePropertyInt("PtTB_Security_Group", -9, 0, GetCurrentUserName(), true)) + ptbcHardColumnCount - 1;
}

CString CPatientToolBar::GetFirstName()
{
	return CString(m_toolBarCombo->GetCurSel()->GetValue(GetFirstNameColumn()).bstrVal);
}

// (z.manning 2008-11-17 10:45) - PLID 31129
CString CPatientToolBar::GetMiddleName()
{
	return CString(m_toolBarCombo->GetCurSel()->GetValue(GetMiddleNameColumn()).bstrVal);
}

CString CPatientToolBar::GetLastName()
{
	return CString(m_toolBarCombo->GetCurSel()->GetValue(GetLastNameColumn()).bstrVal);
}

CString CPatientToolBar::GetFullName()
{
	CString strFullName = GetLastName() + ", " + GetFirstName();
	CString strMiddle = GetMiddleName();
	if(!strMiddle.IsEmpty()) {
		strFullName += " " + strMiddle;
	}

	return strFullName;
}

// (z.manning 2008-11-17 10:49) - PLID 31129
CString CPatientToolBar::GetFullNameByRow(IRowSettingsPtr pRow)
{
	CString strFullName = VarString(pRow->GetValue(GetLastNameColumn()), "") + ", " + 
		VarString(pRow->GetValue(GetFirstNameColumn()), "");
	CString strMiddle = VarString(pRow->GetValue(GetMiddleNameColumn()), "");
	if(!strMiddle.IsEmpty()) {
		strFullName += " " + strMiddle;
	}

	return strFullName;
}

long CPatientToolBar::GetLastPatientID()
{
	return m_LastPatientID;
}

long CPatientToolBar::SafeGetPatientID(long PatientID)
{
	//This function will safely get a patientID

	long NewPatientID;

	NewPatientID = PatientID;

	CString str,strSQL = "";
	if(GetMainFrame()->GetPatientSQL()!="")
		strSQL.Format("AND %s",GetMainFrame()->GetPatientSQL());

	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	str.Format("SELECT ID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE PersonID = {INT} %s",strSQL);

	if(!ReturnsRecordsParam(str, NewPatientID))
		NewPatientID=0;
	
	//TES 1/11/2010 - PLID 36761 - Make sure we can access this patient.
	if(!GetMainFrame()->CanAccessPatient(NewPatientID, true)) {
		NewPatientID = 0;
	}

	if(NewPatientID<=0) {
		//TES 1/11/2010 - PLID 36761 - Select the first patient which isn't in any of the groups which this user doesn't have access to.
		CArray<IdName,IdName> arBlockedGroups;
		GetMainFrame()->GetBlockedGroups(arBlockedGroups);
		CString strIDs;
		for(int i = 0; i < arBlockedGroups.GetSize(); i++) {
			strIDs += FormatString("%li,", arBlockedGroups[i].nID);
		}
		CString strWhere;
		if(!strIDs.IsEmpty()) {
			strIDs.TrimRight(",");
			strWhere.Format("AND ID NOT IN (SELECT PatientID FROM SecurityGroupDetailsT WHERE SecurityGroupID IN (%s))",
				strIDs);
		}
		if(GetMainFrame()->GetPatientSQL()!="")
			strSQL.Format("AND %s %s",GetMainFrame()->GetPatientSQL(), strWhere);
		_RecordsetPtr rs = CreateRecordset("SELECT TOP 1 ID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE ID > 0 %s %s ORDER BY Last ASC, First ASC",strSQL, strWhere);
		if(!rs->eof) {
			NewPatientID = rs->Fields->Item["ID"]->Value.lVal;
		}
	}

	return NewPatientID;
}

// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
void CPatientToolBar::UpdatePatient(long nID)
{	
	try
	{
		//m.hancock - 5/8/2006 - PLID 20462 - Added company name to the query
		// (z.manning 2008-11-12 10:57) - PLID 31129 - Parameterized
		//TES 1/18/2010 - PLID 36895 - Changed this to pull from the GetFromClause() function, which will take blocked groups into account
		// in order to hide demographics as necessary.
		// (j.jones 2010-05-04 10:58) - PLID 32325 - added OHIPHealthCardNum
		// (j.gruber 2010-10-04 14:29) - PLID 40415 - SecurityGroups
		_RecordsetPtr rs = CreateParamRecordset(FormatString("SELECT ComboQ.PersonID, ComboQ.PatientID, "
			"ComboQ.Last AS LastName, ComboQ.First AS FirstName, ComboQ.Middle AS MiddleName, "
			"CASE WHEN BlockedPatientsQ.PatientID Is Null THEN SocialSecurity ELSE Null END AS SocialSecurity, "
			"CASE WHEN BlockedPatientsQ.PatientID Is Null THEN BirthDate ELSE Null END AS BirthDate, "
			"CASE WHEN BlockedPatientsQ.PatientID Is Null THEN Company ELSE Null END AS Company , "
			"CASE WHEN BlockedPatientsQ.PatientID Is Null THEN ComboQ.OHIPHealthCardNum ELSE Null END AS OHIPHealthCardNum, "
			"ComboQ.SecGroupName, "
			"(ComboQ.Last + ', ' + ComboQ.First + ' ' + ComboQ.Middle) AS FullName, "
			"ComboQ.CurrentStatus, ComboQ.Archived,  "
			// (j.gruber 2011-07-22 15:47) - PLID 44118
			" CASE WHEN ComboQ.Archived = 0 THEN CONVERT(int, 0) ELSE CONVERT(int, {INT}) END as ForeColor "
			"FROM %s "
			"WHERE PersonID = {INT}", GetFromClause()), PATIENT_LIST_INACTIVE_COLOR, nID);
		
		IRowSettingsPtr pRow = FindRowByPersonID(nID);

		if (rs->eof)
		{	//patient was deleted
			if (pRow) { //don't remove if not in list

				//if it is the currently selected patient, warn the user
				BOOL bCurrent = FALSE;
				if(m_toolBarCombo->GetCurSel() == pRow) {
					bCurrent = TRUE;

					CChildFrame *p = GetMainFrame()->GetActiveViewFrame();
					if (p) {
						CNxTabView *pView = (CNxTabView *)p->GetActiveView();
						CChildFrame *pFrame = GetMainFrame()->GetActiveViewFrame();
						if (pView && pFrame->IsOfType(PATIENT_MODULE_NAME)) {
							MsgBox("This patient has been deleted by another user.\n"
							"The selection will now be changed to the next patient.");
						}
					}
				}

				//remove the row
				m_rowCache.erase(nID);
				m_toolBarCombo->RemoveRow(pRow);

				// JMJ 6/25/2003: Update the count too
				UpdateStatusText();

				if(bCurrent) {					
					OnSelChosenPatientCombo(m_toolBarCombo->GetCurSel());
				}
			}
		}
		else
		{
			FieldsPtr f = rs->Fields;

			bool bIsCurSel = false;
			if (pRow) {
				if (pRow == m_toolBarCombo->CurSel) {
					bIsCurSel = true;
				} else {
					m_rowCache.erase(nID);
					m_toolBarCombo->RemoveRow(pRow);
					pRow = NULL;
				}
			}

			if (!pRow) {
				pRow = m_toolBarCombo->GetNewRow();
			}

			pRow->Value[ptbcPersonID] = f->Item["PersonID"]->Value;
			// (j.gruber 2011-07-22 15:47) - PLID 44118
			long nForeColor = AdoFldLong(f, "ForeColor", 0);
			if (nForeColor == 0 ) {
				pRow->PutForeColor(0);
			}
			else {
				pRow->PutForeColor(nForeColor);
			}
			pRow->Value[ptbcForeColor] = f->Item["ForeColor"]->Value;
			pRow->Value[GetMainFrame()->m_patToolBar.GetLastNameColumn()] = f->Item["LastName"]->Value;
			pRow->Value[GetMainFrame()->m_patToolBar.GetFirstNameColumn()] = f->Item["FirstName"]->Value;
			pRow->Value[GetMainFrame()->m_patToolBar.GetMiddleNameColumn()] = f->Item["MiddleName"]->Value;
			long nPatientID = f->Item["PatientID"]->Value;
			pRow->Value[GetMainFrame()->m_patToolBar.GetPatientIDColumn()] = nPatientID;
			pRow->Value[GetMainFrame()->m_patToolBar.GetBirthDateColumn()] = f->Item["BirthDate"]->Value;
			pRow->Value[GetMainFrame()->m_patToolBar.GetSSNColumn()] = f->Item["SocialSecurity"]->Value;
			// (j.jones 2010-05-04 10:54) - PLID 32325 - update the OHIP Health Card Number column, even if it is hidden
			pRow->Value[GetMainFrame()->m_patToolBar.GetOHIPHealthCardColumn()] = f->Item["OHIPHealthCardNum"]->Value;
			// (j.gruber 2010-10-04 14:30) - PLID 40415 - Security Groups
			pRow->Value[GetMainFrame()->m_patToolBar.GetSecurityGroupColumn()] = f->Item["SecGroupName"]->Value;
			//m.hancock - 5/8/2006 - PLID 20462 - Add the company name as an option to the new patient toolbar
			pRow->Value[GetMainFrame()->m_patToolBar.GetCompanyColumn()] = f->Item["Company"]->Value;
			_bstr_t bstrFullName = f->Item["FullName"]->Value;
			// (z.manning 2008-11-12 10:50) - PLID 31129 - Make sure we include the ID here if that preference is set.
			if(GetRemotePropertyInt("PatientToolbarShowID", 1, 0, "<None>", true) == 1) {
				bstrFullName += " (" + _bstr_t(nPatientID) + ")";
			}
			// (a.walling 2010-05-26 16:00) - PLID 38906 - No more FullName column
			//pRow->Value[ptbcFullName] = bstrFullName;
			pRow->Value[ptbcCurrentStatus] = f->Item["CurrentStatus"]->Value;

			// (a.walling 2013-01-21 16:48) - PLID 54743 - PatientToolBar and ResToolBar are re-sorting in response to tablecheckers even in situations that are unnecessary, which can introduce a significant delay for large lists
			if (!bIsCurSel) {
				// (j.luckoski 2013-03-18 17:02) - PLID 30966 - If a lookup filter is set, do not add a new patient to that lookup filter
				if (!GetMainFrame()->m_patToolBar.m_butFilter.GetCheck()) {
					pRow = m_toolBarCombo->AddRowSorted(pRow, NULL);
				}
			} else {
				//m_toolBarCombo->Sort(); // remove this eventually and have the datalist sort a single row
				// (a.walling 2013-03-07 09:32) - PLID 55502 - The future is now! Use SortSingleRow
				m_toolBarCombo->SortSingleRow(pRow);
				m_toolBarCombo->EnsureRowInView(pRow);
			}

			// CAH 7/6/2001: Update the count too
			UpdateStatusText();
			// (a.walling 2010-08-16 08:28) - PLID 17768 - This function updates a patient; it does not change the selection!
			//DropBreadcrumb(nID);
		}
		rs->Close();
	}
	NxCatchAll("Could not update patient toolbar");
}

BOOL CPatientToolBar::DoesPatientExistInList(long PatID) {
	
	CString str;
	CString strSQL = GetMainFrame()->GetPatientSQL();
	if(strSQL!="")
		strSQL = "AND " + strSQL;
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	str.Format("SELECT ID FROM PatientsT INNER JOIN PersonT ON PatientsT.PersonID = PersonT.ID WHERE PersonID = {INT} %s",strSQL);
	// (z.manning 2008-12-10 15:18) - PLID 32397 - Fixed IsRecordsetEmpty call to prevent text formatting errors
	return ReturnsRecordsParam(str, PatID);
}

void CPatientToolBar::OnTrySetSelFinishedPatientCombo(long nRowEnum, long nFlags) 
{
	if(nRowEnum == -1) {
		// (a.walling 2010-08-16 08:29) - PLID 17768 - "ChangePatient" is now "UpdatePatient"
		UpdatePatient(m_ActivePatientID);
	} else if (nFlags == dlTrySetSelFinishedSuccess) {
		DropBreadcrumb(m_ActivePatientID);
	}
}

short CPatientToolBar::GetActivePatientStatus()
{
	short nStatus = 1; //default to patient

	if (m_toolBarCombo) {
		// The toolbar exists
		IRowSettingsPtr pCurSel = m_toolBarCombo->CurSel;
		if (pCurSel) {
			// There is a current selection so return the status
			nStatus = VarShort(pCurSel->Value[ptbcCurrentStatus], 1);
			if (nStatus != 1 && nStatus != 2 && nStatus != 3)
				return 1;//default to patient
			else
				return nStatus;
		}
	}
	
	return GetExistingPatientStatus(GetActivePatientID());
}

short CPatientToolBar::GetExistingPatientStatus(long nPatientID)
{
	//this will search the datalist for the patient status, which should be faster
	//than querying a recordset
	
	short nStatus = 1; //default to patient

	if (GetMainFrame()) {
		if(m_toolBarCombo) {
			// The toolbar exists
			IRowSettingsPtr pCurSel = FindRowByPersonID(nPatientID, false);
			if (pCurSel) {
				// The patient exisits so return the status
				nStatus = VarShort(pCurSel->Value[ptbcCurrentStatus], 1);
				if (nStatus != 1 && nStatus != 2 && nStatus != 3)
					return 1;//default to patient
				else
					return nStatus;
			}
		}
	}
	
	//fine, query the recordset
	// (a.walling 2010-11-01 12:38) - PLID 40965 - Parameterized
	_RecordsetPtr rs = CreateParamRecordset("SELECT CurrentStatus FROM PatientsT WHERE PersonID = {INT}",nPatientID);
	if(!rs->eof) {
		nStatus = AdoFldShort(rs, "CurrentStatus",1);
		if (nStatus != 1 && nStatus != 2 && nStatus != 3)
			return 1;//default to patient
		else
			return nStatus;
	}

	return nStatus;
}

HBRUSH CPatientToolBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	//HBRUSH hbr = CToolBar::OnCtlColor(pDC, pWnd, nCtlColor);
	//return hbr;
	// (a.walling 2007-11-27 16:49) - PLID 28062 - VS2008 - Use a solid brush for VC6, transparent for VS2008
	// (a.walling 2008-04-14 11:16) - PLID 29642 - No longer necessary due to overriden DockBar (NxDockBar)
	pDC->SetBkMode(TRANSPARENT);

	return (HBRUSH)::GetStockObject(HOLLOW_BRUSH);

//	return CToolBar::OnCtlColor(pDC, pWnd, nCtlColor);
}

// (a.walling 2008-08-20 15:48) - PLID 29642 - We need to custom draw since by removing the FLAT style
// to get rid of separators also gets rid of transparent buttons, leading to ugly raised buttons. Ew.
// So custom draw is our only option to draw correctly now. Unfortunately you cannot custom draw
// separators, or this would have been much simpler.
BOOL CPatientToolBar::OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* result)
{
	LPNMTBCUSTOMDRAW pDraw = (LPNMTBCUSTOMDRAW)pNotifyStruct;
	BOOL bHandled = FALSE;

	switch(pDraw->nmcd.dwDrawStage) {
	case CDDS_PREPAINT:
		*result = CDRF_NOTIFYITEMDRAW|CDRF_DODEFAULT;
		bHandled = TRUE;
		break;
	case CDDS_ITEMPREPAINT: {

		Bitmap* pBitmap = NULL;

		// (a.walling 2008-04-23 15:50) - PLID 29768 - Modify rects to have a 0,0 origin for the buffer drawing.
		CRect rcScreen = pDraw->nmcd.rc;
		CRect rcArea(0, 0, rcScreen.Width(), rcScreen.Height());

		CRect rcIcon = rcArea;
	
		CRect rcAdjust = rcIcon;
		rcAdjust.DeflateRect(1, 1, 1, 1);

		// which toolbar icon is this?
		switch(pDraw->nmcd.dwItemSpec) {
			// main toolbar
			case ID_GOTO_PREVIOUS_PATIENT: {
				pBitmap = NxGdi::BitmapFromPNGResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_PNG_TBARROWLEFT), 24);
				break;
			}
			case ID_GOTO_NEXT_PATIENT: {
				pBitmap = NxGdi::BitmapFromPNGResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_PNG_TBARROWRIGHT), 24);
				break;
			}
			case ID_BUTTON_BREADCRUMBS: {
				// (a.walling 2010-05-21 10:12) - PLID 17768 - History icon for breadcrumbs
				pBitmap = NxGdi::BitmapFromPNGResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_PNG_TBHISTORY), 24);
				break;
			}
			default: {
				*result = CDRF_DODEFAULT;
				bHandled = FALSE;

				return bHandled;
			}
		};

		if (pBitmap) {

			WORD nFlatState = 0;

			if(pDraw->nmcd.uItemState & CDIS_SELECTED) {
				// item is selected, and most likely also hot
				nFlatState = CDIS_SELECTED;
			} else if (pDraw->nmcd.uItemState & CDIS_HOT) {
				nFlatState = CDIS_HOT;
				// item is hot (mouseover)
			} else if ( (pDraw->nmcd.uItemState & CDIS_GRAYED) || (pDraw->nmcd.uItemState & CDIS_DISABLED) ) {
				nFlatState = CDIS_GRAYED;
				// gray
			} else if ((m_nLastMouseOverIx == tbLeft && pDraw->nmcd.dwItemSpec == ID_GOTO_PREVIOUS_PATIENT) || (m_nLastMouseOverIx == tbRight && pDraw->nmcd.dwItemSpec == ID_GOTO_NEXT_PATIENT) ) {
				// (a.walling 2008-08-20 15:50) - PLID 29642 - If we have a valid LastMouseOverIx, we know we are hot tracking ourselves,
				// and therefore need to force the HOT state
				nFlatState = CDIS_HOT;
			} else if ( (pDraw->nmcd.uItemState == 0) || (pDraw->nmcd.uItemState == CDIS_DEFAULT) || (pDraw->nmcd.uItemState == CDIS_INDETERMINATE) ) {
				nFlatState = CDIS_DEFAULT;
				// normal (default/indeterminate)
			}
			

			if	(
					(pDraw->nmcd.uItemState & CDIS_CHECKED) ||
					(pDraw->nmcd.uItemState & CDIS_FOCUS) ||
					(pDraw->nmcd.uItemState & CDIS_MARKED)
				) {
				ASSERT(FALSE); // We should not have these flags; if we do, figure out how to handle them.
			}

			if (!m_bThemeInit) {
				m_uxtTheme.OpenThemeData(GetSafeHwnd(), "Toolbar");
				m_bThemeInit = TRUE;
			}
			
			// (a.walling 2008-04-23 15:48) - PLID 29768 - Prepare an offscreen buffer with no alpha needed
			Bitmap bmpBuffer(rcArea.Width(), rcArea.Height(), PixelFormat32bppRGB);
			Graphics g(&bmpBuffer);
			
			HDC hDC = g.GetHDC();
			// (a.walling 2008-06-17 16:31) - PLID 30420 - Erase the background onto the buffer
			// this supports the inability for remote users to handle alpha blending and keeps
			// all drawing on the offscreen buffer
			{
				// (a.walling 2009-01-14 13:33) - PLID 32734 - Delegate background erasing to the parent
				CWnd* pWndParent = GetParent();
				CPoint pt(rcScreen.left, rcScreen.top);
				CPoint ptNew(pt);

				// We need to offset the origin so the background is erased correctly. This was never an issue
				// with solid color fills, but now the background may be different depending on it's Y value.
				MapWindowPoints(pWndParent, &ptNew, 1);
				int nOffsetY = pt.y - ptNew.y;

				CPoint ptOld;
				::SetBrushOrgEx(hDC, 0, nOffsetY, &ptOld);
				
				::SendMessage(pWndParent->m_hWnd, WM_ERASEBKGND, (WPARAM)hDC, 0);

				::SetBrushOrgEx(hDC, ptOld.x, ptOld.y, NULL);
			}

			// (a.walling 2008-04-23 15:49) - PLID 29768 - Draw the theme background initially directly to the screen
			if (nFlatState == CDIS_SELECTED) {
				// offset the rect and continue
				rcAdjust.OffsetRect(1, 1);
			}

			// (a.walling 2008-06-17 16:32) - PLID 30420 - Draw the focus state onto the offscreen buffer
			if (nFlatState == CDIS_HOT || nFlatState == CDIS_SELECTED) {
				// Draw the hot focus rect
				if (m_uxtTheme.IsOpen()) {
					m_uxtTheme.DrawThemeBackground(hDC, TP_BUTTON, nFlatState == CDIS_SELECTED ? TS_PRESSED : TS_HOT, rcIcon, NULL);
				} else {
					CDC dc;
					dc.Attach(hDC);

					dc.Draw3dRect(rcIcon, GetSysColor(COLOR_3DSHADOW), GetSysColor(COLOR_3DHILIGHT));

					dc.Detach();
				}
			}
			g.ReleaseHDC(hDC);

			g.SetSmoothingMode(SmoothingModeHighQuality);
			g.SetInterpolationMode(InterpolationModeHighQuality);

			// (a.walling 2008-08-20 15:50) - PLID 29642 - We don't want to stretch, just center
			// the bitmap in the rect we are given
			long nOffsetX = 0, nOffsetY = 0;
			nOffsetX = (rcAdjust.Width() - pBitmap->GetWidth()) / 2;
			nOffsetY = (rcAdjust.Height() - pBitmap->GetHeight()) / 2;

			ASSERT(nOffsetX >= 0);
			ASSERT(nOffsetY >= 0);

			rcAdjust.left += nOffsetX;
			rcAdjust.top += nOffsetY;
			rcAdjust.right = rcAdjust.left + pBitmap->GetWidth();
			rcAdjust.bottom = rcAdjust.top + pBitmap->GetHeight();

			switch(nFlatState) {
				case CDIS_SELECTED:
				case CDIS_HOT:
					{
						{
							// draw a special sphere for hot images
							GraphicsPath path;

							Rect rect = NxGdi::RectFromCRect(rcIcon);
							NxGdi::CreateRoundedRectanglePath(path, rect, 20);

							PathGradientBrush brush(&path);
				
							brush.SetWrapMode(WrapModeClamp);

							brush.SetFocusScales(1 - (35.0f / (REAL)rect.Width), 1 - (35.0f / (REAL)rect.Height));

							Color colors[2] = {
								Color(Color::Transparent),
								NxGdi::ColorFromArgb(0xA0, Color::Lavender),
							};

							REAL positions[2] = {
								0.0f,
								1.0f
							};

							brush.SetInterpolationColors(colors, positions, 2);

							g.FillPath(&brush, &path);
						}						

						g.DrawImage(pBitmap, NxGdi::RectFromCRect(rcAdjust), 0, 0, pBitmap->GetWidth(), pBitmap->GetHeight(), UnitPixel, NULL);
					}
					break;
				case CDIS_DEFAULT: {

					/*
					// desaturate slightly, full saturation on mouseover.
					float fSaturation = 0.75f; // 0 - 1.0

					float fComplement = 1.0f - fSaturation;

					// adjust for luminance of primary colors
					float fComplementR = 0.3086f * fComplement;
					float fComplementG = 0.6094f * fComplement;
					float fComplementB = 0.0820f * fComplement;

					REAL matrix[5][5] = { {fComplementR + fSaturation,  fComplementR,  fComplementR,  0.0f, 0.0f},
						{fComplementG,  fComplementG + fSaturation,  fComplementG,  0.0f, 0.0f},
						{fComplementB,  fComplementB,  fComplementB + fSaturation,  0.0f, 0.0f},
						{0.0f,  0.0f,  0.0f,  1.0f,  0.0f},
						{0.0f,  0.0f,  0.0f,  0.0f,  1.0f}
					};
					
					ColorMatrix cm;
					memcpy(cm.m, matrix, sizeof(REAL)*5*5);

					ImageAttributes ia;
					ia.SetColorMatrix(&cm);
					*/

					// actually don't desaturate

					g.DrawImage(pBitmap, NxGdi::RectFromCRect(rcAdjust), 0, 0, pBitmap->GetWidth(), pBitmap->GetHeight(), UnitPixel, NULL);
					break;
				}
				case CDIS_GRAYED: {
					//Gilles Khouzams colour corrected grayscale shear
					/*REAL matrix[5][5] =	{	{0.3f,0.3f,0.3f,0,0},
											{0.59f,0.59f,0.59f,0,0},
											{0.11f,0.11f,0.11f,0,0},
											{0,0,0,1,0},
											{0,0,0,0,1} };*/

					float fContrast = 0.5f;

					ColorMatrix cm = {
						fContrast,0.1f,0.1f,0,0,
						0.1f,fContrast,0.1f,0,0,
						0,0,fContrast,0,0,
						0,0,0,1,0,
						0.001f,0.001f,0.001f,0,1
					};

					ImageAttributes ia;
					ia.SetColorMatrix(&cm);
					
					g.DrawImage(pBitmap, NxGdi::RectFromCRect(rcAdjust), 0, 0, pBitmap->GetWidth(), pBitmap->GetHeight(), UnitPixel, &ia);
					break;
				}
				default:
					ASSERT(FALSE);
					break;
			}

			delete pBitmap;
			
			// (a.walling 2008-04-23 15:49) - PLID 29768 - Finally, blit the buffer to the screen
			/*Graphics gScreen(pDraw->nmcd.hdc);
			gScreen.DrawImage(&bmpBuffer, rcScreen.left, rcScreen.top);*/

			
			// (a.walling 2009-01-14 14:08) - PLID 32734 - Use GDI primitives
			
			BOOL bBlitSuccess = FALSE;
			
			// (a.walling 2009-08-12 16:12) - PLID 35136 - Realize the palette to draw properly in indexed color situations.
			HDC hdcBuffer = ::CreateCompatibleDC(pDraw->nmcd.hdc);
			if (hdcBuffer) {
				HBITMAP hbmpBuffer = NULL;
				if (Gdiplus::Ok == bmpBuffer.GetHBITMAP(Color::White, &hbmpBuffer)) {
					HBITMAP hOldBitmap = (HBITMAP)::SelectObject(hdcBuffer, hbmpBuffer);

					HPALETTE hPal = (HPALETTE)theApp.m_palette.GetSafeHandle();
					HPALETTE hOld = ::SelectPalette(pDraw->nmcd.hdc, hPal, FALSE);
					::RealizePalette(pDraw->nmcd.hdc);
					bBlitSuccess = ::BitBlt(pDraw->nmcd.hdc, rcScreen.left, rcScreen.top, rcScreen.Width(), rcScreen.Height(), hdcBuffer, 0, 0, SRCCOPY);
					::SelectPalette(pDraw->nmcd.hdc, hOld, FALSE);

					::SelectObject(hdcBuffer, hOldBitmap);
					::DeleteObject(hbmpBuffer);
				}
				::DeleteDC(hdcBuffer);
			}

			if (!bBlitSuccess) {
				// (a.walling 2008-04-23 15:49) - PLID 29768 - Finally, blit the buffer to the screen
				Graphics gScreen(pDraw->nmcd.hdc);
				gScreen.DrawImage(&bmpBuffer, rcScreen.left, rcScreen.top);
			}
			
			// We just want to do the default for now and leave the framework in place for the future.
			*result = CDRF_SKIPDEFAULT;
			bHandled = TRUE;
		} else {
			// We just want to do the default for now and leave the framework in place for the future.
			*result = CDRF_DODEFAULT;
			bHandled = FALSE;
		}


		break;
	}
	default:
		*result = CDRF_DODEFAULT;
		bHandled = FALSE;
		break;
	}

	return bHandled;
}

// (a.walling 2008-08-20 15:51) - PLID 29642 - I was ignoring this, but it is best to let the toolbar erase the background.
// This may lead to some flickering but since we have other controls on the toolbar it is the simplest way.
BOOL CPatientToolBar::OnEraseBkgnd(CDC* pDC)
{
	// (a.walling 2009-01-14 13:33) - PLID 32734 - Delegate background erasing to the parent
	CWnd* pWndParent = GetParent();

	CRect rcScreen;
	GetWindowRect(rcScreen);
	CPoint pt(rcScreen.left, rcScreen.top);
	CPoint ptNew(pt);

	// We need to offset the origin so the background is erased correctly. This was never an issue
	// with solid color fills, but now the background may be different depending on it's Y value.

	MapWindowPoints(pWndParent, &ptNew, 1);
	int nOffsetY = pt.y - ptNew.y;

	CPoint ptOld = pDC->SetBrushOrg(0, nOffsetY);
	pWndParent->SendMessage(WM_ERASEBKGND, (WPARAM)pDC->GetSafeHdc(), 0);
	pDC->SetBrushOrg(ptOld.x, ptOld.y);

	return TRUE;
}

// (a.walling 2008-08-20 15:52) - PLID 29642 - Caveat of removing FLAT style #2: no hot tracking. Argh! If Visual Styles
// are enabled, then we will have hot tracking by default, awesome. However we must support it when they are not enabled.
// Therefore we must handle the hot tracking ourselves.
void CPatientToolBar::OnMouseMove(UINT nFlags, CPoint point)
{
	if (!m_bThemeInit) {
		m_uxtTheme.OpenThemeData(GetSafeHwnd(), "Toolbar");
		m_bThemeInit = TRUE;
	}
	// (a.walling 2008-08-20 16:05) - PLID 29642 - This is only necessary is Visual Styles are not enabled.
	if (!m_uxtTheme.IsOpen()) {
		CToolBarCtrl& tb = GetToolBarCtrl();

		long nIndex = tb.HitTest(&point);

		if (nIndex != m_nLastMouseOverIx) {
			// (a.walling 2008-08-20 15:53) - PLID 29642 - The cursor is pointing elsewhere now, so
			// we need to update our display. We only need to worry if either the old or new button
			// is one that we are handling hot tracking for ourselves. Just invalidate them so they
			// will be redrawn correctly during the next paint cycle.
			CRect rcNew, rcOld;
			rcNew.SetRectEmpty();
			rcOld.SetRectEmpty();

			if (nIndex == tbLeft || nIndex == tbRight) {
				GetItemRect(nIndex, rcNew);
			}

			if (m_nLastMouseOverIx == tbLeft || m_nLastMouseOverIx == tbRight) {
				GetItemRect(m_nLastMouseOverIx, rcOld);
			}
			
			m_nLastMouseOverIx = nIndex;
			
			if (!rcNew.IsRectEmpty()) {
				InvalidateRect(rcNew);
			}
			if (!rcOld.IsRectEmpty()) {
				InvalidateRect(rcOld);
			}
		}

		m_nLastMouseOverIx = nIndex;

		TRACKMOUSEEVENT me;
		me.cbSize = sizeof(TRACKMOUSEEVENT);
		me.dwFlags = TME_LEAVE;
		me.hwndTrack = GetSafeHwnd();
		_TrackMouseEvent(&me);
		m_bTrackingMouse = TRUE;
	}

	CToolBar::OnMouseMove(nFlags, point);
}

LRESULT CPatientToolBar::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	if (m_bTrackingMouse && (m_nLastMouseOverIx == tbLeft || m_nLastMouseOverIx == tbRight)) {
		// (a.walling 2008-08-20 15:54) - PLID 29642 - We are leaving the window, so make sure any button
		// that is drawn 'hot' is redrawn as normal.
		CRect rc;
		GetItemRect(m_nLastMouseOverIx, rc);

		m_nLastMouseOverIx = -LONG_MIN;
		
		if (!rc.IsRectEmpty()) {
			InvalidateRect(rc);
		}
	}

	Default();
	return 0;
}

void CPatientToolBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	try {
		// (a.walling 2008-08-21 14:06) - PLID 31056 - This bit of a hack is for non-composited, non-themed drawing issues
		// with tooltips. Basically we just find out where the tooltip last was, and invalidate that portion of the window.
		// The toolbar (with CBRS_TOOLTIPS style) will use MFC's internal CToolTipCtrl, which is a per-thread object.

		// (a.walling 2008-10-08 15:09) - PLID 31575 - m_pToolTip (common control tooltip) is now in AFX_MODULE_THREAD_STATE
		AFX_MODULE_THREAD_STATE* pThreadState = AfxGetModuleThreadState();
		CToolTipCtrl* pToolTip = pThreadState->m_pToolTip;
		if (pToolTip->GetSafeHwnd()) { // this is also, implicitly, a check for null.
			CRect rc;
			rc.SetRectEmpty();

			pToolTip->GetWindowRect(rc);
			ScreenToClient(rc);

			CRect rcClient;
			GetClientRect(rcClient);

			CRect rcInvalid;

			if (rcInvalid.IntersectRect(rcClient, rc)) {
				//RedrawWindow(rcInvalid, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ERASENOW | RDW_ALLCHILDREN);
				//just invalidate so we can redraw during the next paint cycle
				InvalidateRect(rcInvalid);
			}
		}
	} NxCatchAllCallIgnore({LogDetail("Error handling patient toolbar OnLButtonUp");};);

	CToolBar::OnLButtonUp(nFlags, point);
}

// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing (NcPaint)
void CPatientToolBar::OnNcPaint()
{
	// (a.walling 2012-03-02 10:43) - PLID 48589 - No more toolbar borders
	/*
	if (g_bClassicToolbarBorders) {
		CToolBar::OnNcPaint();
	}
	*/
	return;
}

//TES 1/5/2010 - PLID 36761 - Some new functions, now that the toolbar and patient IDs are protected

//TES 1/5/2010 - PLID 36761 - Returns the variant representing the value in the indicated column, for the currently selected row.
_variant_t CPatientToolBar::GetCurrentlySelectedValue(short nCol)
{
	return m_toolBarCombo->GetCurSel()->GetValue(nCol);
}

//TES 1/5/2010 - PLID 36761 - Sets the indicated column to have the given value, for the currently selected row.
void CPatientToolBar::SetCurrentlySelectedValue(short nCol, const _variant_t &var)
{
	IRowSettingsPtr pCurSel = m_toolBarCombo->CurSel;
	if (pCurSel) {
		pCurSel->PutValue(nCol, var);
		// (a.walling 2013-03-07 09:32) - PLID 55502 - The future is now! Use SortSingleRow
		m_toolBarCombo->SortSingleRow(pCurSel);
		m_toolBarCombo->EnsureRowInView(pCurSel);
		UpdateStatusText();
	}
}

//TES 1/5/2010 - PLID 36761 - Sets the indicated column to have the given value, for the row represented by the given patient ID.
void CPatientToolBar::SetValueByPersonID(long nPersonID, short nCol, const _variant_t &var)
{
	IRowSettingsPtr pRow = FindRowByPersonID(nPersonID);
	if (pRow) {
		pRow->PutValue(nCol, var);
		// (a.walling 2013-03-07 09:32) - PLID 55502 - The future is now! Use SortSingleRow
		m_toolBarCombo->SortSingleRow(pRow);
		if (pRow == m_toolBarCombo->CurSel) {
			m_toolBarCombo->EnsureRowInView(pRow);
		}
		UpdateStatusText();
	}
}

//TES 1/5/2010 - PLID 36761 - Row position accessors
// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
IRowSettingsPtr CPatientToolBar::GetCurrentlySelectedRow()
{
	return m_toolBarCombo->CurSel;
}

// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
IRowSettingsPtr CPatientToolBar::GetFirstRow()
{
	return m_toolBarCombo->GetFirstRow();
}

// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
IRowSettingsPtr CPatientToolBar::GetLastRow()
{
	return m_toolBarCombo->GetLastRow();
}

long CPatientToolBar::GetRowCount()
{
	return m_toolBarCombo->GetRowCount();
}

//TES 1/5/2010 - PLID 36761 - Dropdown state accessors.
BOOL CPatientToolBar::IsDroppedDown()
{
	return VarShort(m_toolBarCombo->DropDownState) == 0 ? FALSE : TRUE;
}

void CPatientToolBar::SetDroppedDown(BOOL bDroppedDown)
{
	m_toolBarCombo->DropDownState = bDroppedDown?VARIANT_TRUE:VARIANT_FALSE;
}

//TES 1/5/2010 - PLID 36761 - Removes the currently selected row from the list, including setting the new selection.
void CPatientToolBar::RemoveCurrentRow()
{
	IRowSettingsPtr pRow = m_toolBarCombo->GetCurSel();
	IRowSettingsPtr pRowNext = NULL;

	if (pRow) {
		pRowNext = pRow->GetNextRow();
		if (!pRowNext) {
			pRowNext = pRow->GetPreviousRow();
		}
		if (!pRowNext) {
			pRowNext = m_toolBarCombo->GetLastRow();
		}
	}

	if (pRow) {
		m_rowCache.erase((long)pRow->Value[0]);
		m_toolBarCombo->RemoveRow(pRow);
		m_toolBarCombo->PutCurSel(pRowNext);
	}
	m_LastPatientID = -1;

	CMainFrame *pMainFrame = GetMainFrame();
	if(m_toolBarCombo->GetCurSel()) {
		//PLID 13172 - it's possible we just deleted the last patient
		//TES 1/11/2010 - PLID 36761 - Call SafeGetPatientID() in case we don't have access to this person.
		m_ActivePatientID = SafeGetPatientID(pRowNext->GetValue(ptbcPersonID));
		//TES 1/21/2010 - PLID 36761 - Need to make sure we reflect the new selection in the list.		
		long nTrySetSelResult = m_toolBarCombo->TrySetSelByColumn_Deprecated(0,(long)m_ActivePatientID);
		// (a.walling 2010-06-29 07:51) - PLID 17768 - TrySetSel may return immediately without firing an event; ensure we handle any changes.
		if (nTrySetSelResult >= 0) {
			DropBreadcrumb(m_ActivePatientID);
		}
		UpdateStatusText();
		if(pMainFrame) {
			pMainFrame->HandleActivePatientChange(); // (z.manning 2009-05-19 12:38) - PLID 28512
		}
	}
	else {
		if(pMainFrame) {
			//PLID 13172 - we need to refresh the list, we're out of folks
			ResetFilter();
			pMainFrame->HandleFilterSearchClicked();
		}
	}
}

//TES 1/5/2010 - PLID 36761 - Removes the specified row from the list.
void CPatientToolBar::RemoveRowByPersonID(long nPersonID)
{
	IRowSettingsPtr pRow = FindRowByPersonID(nPersonID);
	if(!pRow) {
		return;
	}
	if(pRow == m_toolBarCombo->CurSel) {
		//TES 1/5/2010 - PLID 36761 - This is the current row, so call that function, which will also make sure the new patient is
		// properly selected, etc.
		RemoveCurrentRow();
		return;
	}
	else {
		m_rowCache.erase(nPersonID);
		m_toolBarCombo->RemoveRow(pRow);
		UpdateStatusText();
	}
}

//TES 1/6/2010 - PLID 36761 - Encapsulated in a function, this was being called a bunch of places, in this file and others.
void CPatientToolBar::UpdateStatusText()
{
	if(!IsLoading()) {
		long nCurSelIndex = 0;
		IRowSettingsPtr pCurSelRow = m_toolBarCombo->GetCurSel();
		if (pCurSelRow) {
			nCurSelIndex = pCurSelRow->CalcRowNumber() + 1;
		}
		CString str;
		str.Format("%i / %i", nCurSelIndex, m_toolBarCombo->GetRowCount());
		m_text3.SetWindowText(str);
	}
}

//TES 1/5/2010 - PLID 36761 - Ways to change the currently selected patient.
bool CPatientToolBar::SelectByRow(IRowSettingsPtr pRow)
{
	// (j.jones 2012-08-08 09:28) - PLID 51063 - added a try/catch
	try {

		long nPatientID = VarLong(pRow->GetValue(ptbcPersonID));
		//TES 1/11/2010 - PLID 36761 - Make sure we have access.
		if(!GetMainFrame()->CanAccessPatient(nPatientID, false)) {
			return false;
		}

		m_toolBarCombo->PutCurSel(pRow);

		m_LastPatientID = m_ActivePatientID;
		m_ActivePatientID = VarLong(m_toolBarCombo->GetCurSel()->GetValue(ptbcPersonID));
		DropBreadcrumb(m_ActivePatientID);

		CMainFrame *pMainFrame = GetMainFrame();
		if (pMainFrame)
		{	pMainFrame->SetStatusButtons();
			if (pMainFrame->GetActiveView())
				pMainFrame->GetActiveView()->UpdateView();

			pMainFrame->HandleActivePatientChange(); // (z.manning 2009-05-19 12:38) - PLID 28512
		}

		OnIdleUpdateCmdUI(TRUE, 0L);//disable/enable

		return true;

	}NxCatchAll(__FUNCTION__);

	return false;
}

// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
bool CPatientToolBar::SelectFirstRow()
{
	return SelectByRow(m_toolBarCombo->GetFirstRow());
}

// (a.walling 2010-10-28 15:30) - PLID 41183 - Patient Dropdown now an NxDataList2
bool CPatientToolBar::SelectLastRow()
{
	return SelectByRow(m_toolBarCombo->GetLastRow());
}
	
bool CPatientToolBar::SelectLastPatient()
{
	// (j.jones 2012-08-08 09:28) - PLID 51063 - added a try/catch
	try {

		long lastPatient = m_LastPatientID;
		if(lastPatient==-1) {
			lastPatient = m_TempLastPatientID;
		}

		if(lastPatient==-1) {
			AfxMessageBox("There is no previous patient to switch to.");
			return false;
		}

		//TES 1/11/2010 - PLID 36761 - Make sure we have access.
		if(!GetMainFrame()->CanAccessPatient(lastPatient,false)) {
			return false;
		}

		if(NXDATALIST2Lib::IRowSettingsPtr pRow = FindRowByPersonID(lastPatient)) {
			m_LastPatientID = GetActivePatientID();
			m_toolBarCombo->CurSel = pRow;
			m_toolBarCombo->EnsureRowInView(pRow);
			m_ActivePatientID = lastPatient;
			DropBreadcrumb(m_ActivePatientID);
		}
		else {
			AfxMessageBox("There is no previous patient to switch to.");
			return false;
		}

		CMainFrame *pMainFrame = GetMainFrame();
		if (pMainFrame)
		{	pMainFrame->SetStatusButtons();
			if (pMainFrame->GetActiveView())
				pMainFrame->GetActiveView()->UpdateView();

			pMainFrame->HandleActivePatientChange(); // (z.manning 2009-05-19 12:38) - PLID 28512
		}

		return true;

	}NxCatchAll(__FUNCTION__);

	return false;
}

//TES 1/6/2010 - PLID 36761 - Copied here, was a static global function in MainFrm.cpp
void CPatientToolBar::ResetFilter()
{	
	//TES 1/25/2010 - PLID 36761 - Reworded this message, we can get here even with a non-empty search set.
	AfxMessageBox ("Either no patients met the specified criteria, or you do not have access to any of the patients which meet the specified criteria.  "
		"The search will be reset to all patients");
	m_butFilter.SetCheck(false);
	m_butActive.SetCheck(false);
	m_butAll.SetCheck(true);
	m_butPatientSearchCheck.SetCheck(false);
	m_butAllSearchCheck.SetCheck(true);
	m_butProspectSearchCheck.SetCheck(false);
}

//TES 1/6/2010 - PLID 36761 - Is the datalist currently loading?
bool CPatientToolBar::IsLoading()
{
	CString str;
	m_text3.GetWindowText(str);
	if(str == "Loading...") {
		return true;
	}
	else {
		return false;
	}
}

// (c.haag 2010-01-13 13:32) - PLID 20973 - Structure and sort function used for 
// last,first column sorting
typedef struct {
	short nColumnID;
	short nSortOrder;
} SortColumn;

static int CompareSortColumns(const void *pDataA, const void *pDataB)
{
	const SortColumn *pa = (const SortColumn*)pDataA;
	const SortColumn *pb = (const SortColumn*)pDataB;

	if (pa->nSortOrder < pb->nSortOrder) {
		return -1;
	}
	else if (pa->nSortOrder > pb->nSortOrder) {
		return 1;
	}
	else {
		return 0;
	}
}

// (j.jones 2013-02-15 10:41) - PLID 40804 - moved the last/first sorting to its own function
// Takes in the nCol you're sorting on, returns TRUE if the function did sort, which means
// the caller should not continue with its own sort.
BOOL CPatientToolBar::ApplySortLastFirstAtOnce(short nCol)
{
	if(GetRemotePropertyInt("PatientToolbarSortLastFirstAtOnce", 1, 0, GetCurrentUserName(), true) == 1) {

		short iLastNameCol = GetLastNameColumn();
		short iFirstNameCol = GetFirstNameColumn();

		// If we get here, the user clicked on the last name column, and intends to do a secondary
		// sort on first name. 
		//
		// Reorder the column sort priorities such that the last name is first, first name is second,
		// and everything else is relative to where they are right now. We also need to flip the sort
		// order if the user clicked on the primary sort column. All in all, we're completly taking over
		// the sort operation here.
		//
		CArray<SortColumn,SortColumn&> aSortColumns;

		// Build an array of column indices and sort priorities for items that have a sort priority
		short i, nColumns = m_toolBarCombo->GetColumnCount();
		SortColumn scPrimary;
		scPrimary.nSortOrder = -1;
		for (i=0; i < nColumns; i++) {
		
			IColumnSettingsPtr pCol = m_toolBarCombo->GetColumn(i);
			SortColumn sc;
			sc.nColumnID = i;
			sc.nSortOrder = pCol->GetSortPriority();

			// Add this column to the sort array if it's anything but the first or last name
			if (i != iLastNameCol && i != iFirstNameCol) {
				if (sc.nSortOrder > -1) {
					aSortColumns.Add(sc);
				}
			}

			// Find the column with the lowest non-negative sort order. This is the primary sort column.
			if (-1 == scPrimary.nSortOrder || (sc.nSortOrder >= 0 && sc.nSortOrder < scPrimary.nSortOrder)) {
				scPrimary = sc;
			}
		}

		// At this point, the array now looks something like:
		//
		// ColID = 5  SortID = 2
		// ColID = 6  SortID = 4
		// ColID = 7  SortID = 3
		//
		// Now we sort the array from highest to lowest priority (0 = top priority, > 0 = lower)
		//
		if (aSortColumns.GetSize() > 0) {
			qsort(aSortColumns.GetData(), aSortColumns.GetSize(), sizeof(SortColumn), CompareSortColumns);
		}

		// At this point, the array now looks something like:
		//
		// ColID = 5  SortID = 2
		// ColID = 7  SortID = 3
		// ColID = 6  SortID = 4
		//
		//
		// Now add the last name column to the top of the list, and the first name column to the second
		// element in the list.
		//
		SortColumn sc;
		sc.nColumnID = iLastNameCol;
		aSortColumns.InsertAt(0, sc);
		sc.nColumnID = iFirstNameCol;
		aSortColumns.InsertAt(1, sc);

		//*************************************************************//
		// Update sort priorities
		//*************************************************************//
		// At this point, the array now looks something like:
		//
		// ColID = 3  SortID = <undefined>
		// ColID = 4  SortID = <undefined>
		// ColID = 5  SortID = 2
		// ColID = 7  SortID = 3
		// ColID = 6  SortID = 4
		//
		// Now reset all sort priorities in the list
		for (i=0; i < nColumns; i++) {
			IColumnSettingsPtr pCol = m_toolBarCombo->GetColumn(i);
			pCol->PutSortPriority(-1);
		}
		// Now assign priorities based on the array contents. Remember, the array is
		// sorted by datalist column sort priority, and 0 is the highest priority.
		for (i=0; i < aSortColumns.GetSize(); i++) {
			IColumnSettingsPtr pCol = m_toolBarCombo->GetColumn(aSortColumns[i].nColumnID);
			pCol->PutSortPriority(i);
		}

		//*************************************************************//
		// Update sort order
		//*************************************************************//
		// See if the user clicked on the primary sort column.
		BOOL bClickedOnPrimarySort = (nCol == scPrimary.nColumnID) ? TRUE : FALSE;
		if (bClickedOnPrimarySort) {
			//  If so, we have to reverse the sort order for all columns.
			for (i=0; i < nColumns; i++) {
				IColumnSettingsPtr pCol = m_toolBarCombo->GetColumn(i);
				pCol->PutSortAscending( !pCol->GetSortAscending() );
			}						
		}
		else {
			// If not, we have to force the sort for the clicked column to be ascending
			IColumnSettingsPtr pCol = m_toolBarCombo->GetColumn(nCol);
			const _variant_t varTrue(VARIANT_TRUE, VT_BOOL);
			pCol->PutSortAscending(varTrue);
		}

		// Ensure that the first name column has the same sort as the last name column
		IColumnSettingsPtr pColLast = m_toolBarCombo->GetColumn(iLastNameCol);
		IColumnSettingsPtr pColFirst = m_toolBarCombo->GetColumn(iFirstNameCol);
		pColFirst->PutSortAscending( pColLast->GetSortAscending() );

		m_toolBarCombo->Sort();
		//returning true will tell the caller not to sort again
		return TRUE;
	}
	else {
		// Preference is off. Defer to the usual column sort.
	}

	//returning false tells the caller we did not force a sort
	return FALSE;
}

// (c.haag 2010-01-13 09:47) - PLID 20973 - This function takes the event and makes it so if we're
// click on the Last Name column, that we optionally sort by Last, First instead.
void CPatientToolBar::OnColumnClickingPatientCombo(short nCol, BOOL FAR* bAllowSort) 
{
	try {

		// (j.jones 2013-02-15 10:47) - PLID 40804 - save the sort order
		if(nCol != -1) {
			//save the sort order
			SetRemotePropertyInt("PatientToolbarSortColumn", nCol, 0, GetCurrentUserName());
			long nAsc = 1;
			if(m_toolBarCombo->GetColumn(nCol)->GetSortPriority() == 0 && m_toolBarCombo->GetColumn(nCol)->GetSortAscending()) {
				//The sort doesn't change until this function exits, so we have to calculate the sort order.
				//This click would toggle a descending sort only if the column is currently the primary
				//sort, and is currently sorting ascending.
				nAsc = 0;
			}
			SetRemotePropertyInt("PatientToolbarSortAsc", nAsc, 0, GetCurrentUserName());
		}

		if (nCol == GetLastNameColumn()) {
			// (j.jones 2013-02-15 10:41) - PLID 40804 - moved the last/first sorting to its own function
			// Returns TRUE if the function did sort, which means we will have to disable the sort in this function.
			if(ApplySortLastFirstAtOnce(nCol)) {
				// Do not defer to the usual sort, because the usual sort checks to see if the user clicked on
				// the primary sort column *after* we played "musical chairs" with the columns, and behaves
				// accordingly. We don't want this to happen; we are in full control here and we will do the
				// sort now.
				*bAllowSort = FALSE;
			}
		}
		else {
			// User clicked on a column that isn't the Last Name column. Defer to the usual column sort
		}
	}
	NxCatchAll(__FUNCTION__);
}

CString CPatientToolBar::GetFromClause()
{
	// (z.manning 2010-10-28 11:45) - PLID 41129 - Now have a global function for this
	return ::GetPatientFromClause();
}

// (a.walling 2010-05-21 10:12) - PLID 17768 - Handle right mouse click on the toolbar buttons as well
void CPatientToolBar::OnRButtonDown(UINT nFlags, CPoint pt)
{
	if (m_BreadcrumbTrail.GetLength() <= 0) {
		return;
	}

	CToolBarCtrl& tb(GetToolBarCtrl());

	long nIndex = tb.HitTest(&pt);

	if (nIndex >= 0) {
		TBBUTTON tbButton;
		if (tb.GetButton(nIndex, &tbButton)) {
			if (tbButton.idCommand == ID_GOTO_PREVIOUS_PATIENT || tbButton.idCommand == ID_GOTO_NEXT_PATIENT || tbButton.idCommand == ID_BUTTON_BREADCRUMBS) {
				PopupBreadcrumbMenu();
			}
		}
	}
}

// (a.walling 2010-05-21 10:12) - PLID 17768 - Popup the patient breadcrumbs menu
void CPatientToolBar::PopupBreadcrumbMenu()
{
	try {
		CPoint ptScreen, ptTool;
		GetMessagePos(ptScreen);

		CToolBarCtrl& tb(GetToolBarCtrl());

		ptTool = ptScreen;
		tb.ScreenToClient(&ptTool);

		long nIndex = tb.HitTest(&ptTool);

		TPMPARAMS TpmParams;
		::ZeroMemory(&TpmParams, sizeof(TpmParams));
		LPTPMPARAMS pTpmParams = NULL;
		if (nIndex >= 0) {
			CRect rcExclude;
			if (tb.GetItemRect(nIndex, &rcExclude)) {
				tb.ClientToScreen(&rcExclude);
				TpmParams.cbSize = sizeof(TPMPARAMS);
				TpmParams.rcExclude = rcExclude;
				pTpmParams = &TpmParams;
			}
		} 

		CMenu menu;
		CMap<long, long, long, long> mapIDs;
		// (a.walling 2010-06-29 08:11) - PLID 17768 - For safety, pass in the active patient ID
		m_BreadcrumbTrail.CreatePopupMenu(menu, mapIDs, GetActivePatientID());
		int nResult = menu.TrackPopupMenuEx( (!GetSystemMetrics(SM_MENUDROPALIGNMENT) ? TPM_LEFTALIGN : TPM_RIGHTALIGN)|TPM_LEFTBUTTON|TPM_RIGHTBUTTON|TPM_RETURNCMD, ptScreen.x, ptScreen.y, this, pTpmParams);
		if (nResult > 0) {
			long nPatientID = -1;
			if (mapIDs.Lookup(nResult, nPatientID)) {
				if (nPatientID != -1 && nPatientID != GetActivePatientID()) {
					if (GetMainFrame()) {
						// (a.walling 2010-08-18 12:02) - PLID 17768 - Nowhere else should be able to call GotoPatient on a deleted patient, so I will set a flag
						// to explicitly check for deleted patients. I was going to put it here, but decided it was wiser to keep everything centralized.
						GetMainFrame()->GotoPatient(nPatientID, true);
						// (d.lange 2011-03-11 11:38) - PLID 41010 - We've changed patients, so let's refresh the device import dialog if necessary
						GetMainFrame()->NotifyDeviceImportPatientChanged();
					}
				}
			}
		}
	} NxCatchAll(__FUNCTION__);
}

// (a.walling 2010-05-21 10:12) - PLID 17768 - Drop a breadcrumb (keep track of this patient in the history)
void CPatientToolBar::DropBreadcrumb(long nPatientID)
{
	try {
		if (nPatientID > 0) {
			// (a.walling 2010-05-21 10:12) - PLID 17768 - Use the nPatientID here, not the m_ActivePatientID!
			IRowSettingsPtr pRow = FindRowByPersonID(nPatientID);
			if (pRow) {
				CString strFullName = GetFullNameByRow(pRow);
				COleDateTime dtBirthDate = GetPatientBirthDateByRow(pRow);

				CString strDescription = strFullName;
				if (dtBirthDate.GetStatus() == COleDateTime::valid) {
					strDescription += " - ";
					strDescription += FormatDateTimeForInterface(dtBirthDate, 0, dtoDate);
				}

				long nUserDefinedID = GetUserDefinedIDByRow(pRow);
				m_BreadcrumbTrail.Add(nPatientID, nUserDefinedID, strDescription);
			}
		}
	} NxCatchAll("Error remembering selected patient!");
}


CPatientToolBar::CBreadcrumbTrail::CBreadcrumbTrail()
{
}

CPatientToolBar::CBreadcrumbTrail::~CBreadcrumbTrail()
{
	Clear();
}

void CPatientToolBar::CBreadcrumbTrail::Clear()
{
	POSITION pos = m_listBreadcrumbs.GetHeadPosition();
	while (pos) {
		CBreadcrumb* p = m_listBreadcrumbs.GetNext(pos);
		delete p;
	}
	m_listBreadcrumbs.RemoveAll();
}

void CPatientToolBar::CBreadcrumbTrail::Prune()
{
	// (a.walling 2010-05-21 10:12) - PLID 17768 - Set max length
	int nMaxLength = GetRemotePropertyInt("PatientBreadcrumbTrailMax", 20, 0, GetCurrentUserName(), true);
	while (m_listBreadcrumbs.GetSize() > nMaxLength) {
		CBreadcrumb* pOld = m_listBreadcrumbs.RemoveTail();
		delete pOld;
	}
}

int CPatientToolBar::CBreadcrumbTrail::GetLength()
{
	return m_listBreadcrumbs.GetSize();
}

long CPatientToolBar::CBreadcrumbTrail::GetHeadPatientID()
{
	if (!m_listBreadcrumbs.IsEmpty()) {
		CBreadcrumb* pCrumb = m_listBreadcrumbs.GetHead();
		return pCrumb->m_nPatientID;
	}

	return -1;
}

void CPatientToolBar::CBreadcrumbTrail::Add(long nPatientID, long nUserDefinedID, const CString& strDescription)
{
	POSITION pos = m_listBreadcrumbs.GetHeadPosition();
	while (pos) {
		POSITION posCurrent = pos;
		CBreadcrumb* p = m_listBreadcrumbs.GetNext(pos);

		if (p->m_nPatientID == nPatientID) {
			delete p;
			m_listBreadcrumbs.RemoveAt(posCurrent);
		}
	}

	CBreadcrumb* pCrumb = new CBreadcrumb();

	pCrumb->m_nPatientID = nPatientID;
	pCrumb->m_nUserDefinedID = nUserDefinedID;
	GetSystemTimeAsFileTime(&(pCrumb->m_ftDate));
	pCrumb->m_strDescription = strDescription;
	pCrumb->m_strDescription.Replace("\t", " ");
	pCrumb->m_strDescription.Replace("\r", "");
	pCrumb->m_strDescription.Replace("\n", " ");

	m_listBreadcrumbs.AddHead(pCrumb);

	Prune();
}

void CPatientToolBar::CBreadcrumbTrail::AddFromData(long nPatientID, long nUserDefinedID, const FILETIME& ftDate, const CString& strDescription)
{
	POSITION pos = m_listBreadcrumbs.GetHeadPosition();
	while (pos) {
		POSITION posCurrent = pos;
		CBreadcrumb* p = m_listBreadcrumbs.GetNext(pos);

		if (p->m_nPatientID == nPatientID) {
			if (CompareFileTime(&(p->m_ftDate), &ftDate) >= 0) {
				return;
			}
			delete p;
			m_listBreadcrumbs.RemoveAt(posCurrent);
		}
	}

	// create the crumb
	CBreadcrumb* pCrumb = new CBreadcrumb();

	pCrumb->m_nPatientID = nPatientID;
	pCrumb->m_nUserDefinedID = nUserDefinedID;
	pCrumb->m_ftDate = ftDate;
	pCrumb->m_strDescription = strDescription;
	pCrumb->m_strDescription.Replace("\t", " ");
	pCrumb->m_strDescription.Replace("\r", "");
	pCrumb->m_strDescription.Replace("\n", " ");

	// now find the appropriate drop point
	pos = m_listBreadcrumbs.GetHeadPosition();
	while (pos) {
		POSITION posCurrent = pos;
		CBreadcrumb* p = m_listBreadcrumbs.GetNext(pos);

		if (CompareFileTime(&(p->m_ftDate), &(pCrumb->m_ftDate)) <= 0) {
			m_listBreadcrumbs.InsertBefore(posCurrent, pCrumb);
			Prune();
			return;
		}
	}

	m_listBreadcrumbs.AddTail(pCrumb);
	Prune();
}

// (a.walling 2010-05-21 10:12) - PLID 17768 - Create the popup menu with all entries
// (a.walling 2010-06-29 08:11) - PLID 17768 - For safety, pass in the active patient ID
void CPatientToolBar::CBreadcrumbTrail::CreatePopupMenu(CMenu& menu, CMap<long, long, long, long>& mapIDs, long nActivePatientID)
{
	menu.CreatePopupMenu();

	long nCurrentID = 42;	
	long nCurrentRowPosition = 0;

	nCurrentID++;
	menu.AppendMenu(MF_DISABLED, nCurrentID, "Patient ID");
	nCurrentID++;
	menu.AppendMenu(MF_SEPARATOR, nCurrentID, "");

	POSITION pos;
	
	pos = m_listBreadcrumbs.GetHeadPosition();
	nCurrentRowPosition = 0;
	while (pos) {
		CBreadcrumb* pCrumb = m_listBreadcrumbs.GetNext(pos);
		nCurrentID++;
		nCurrentRowPosition++;

		CString strUserDefinedID;
		strUserDefinedID.Format("%li", pCrumb->m_nUserDefinedID);

		UINT uFlags = MF_STRING;

		// (a.walling 2010-06-29 08:11) - PLID 17768 - Check only if ID matches
		if (nActivePatientID != -1 && nActivePatientID == pCrumb->m_nPatientID) {
			uFlags |= MF_CHECKED;
		}

		menu.AppendMenu(uFlags, nCurrentID, strUserDefinedID);

		mapIDs[nCurrentID] = pCrumb->m_nPatientID;
	}

	nCurrentID++;
	menu.AppendMenu(MF_MENUBREAK|MF_DISABLED, nCurrentID, "Name");
	nCurrentID++;
	menu.AppendMenu(MF_SEPARATOR, nCurrentID, "");

	pos = m_listBreadcrumbs.GetHeadPosition();
	nCurrentRowPosition = 0;
	while (pos) {
		CBreadcrumb* pCrumb = m_listBreadcrumbs.GetNext(pos);
		nCurrentID++;
		nCurrentRowPosition++;

		CString strMenuDescription = pCrumb->m_strDescription;
		// (j.jones 2011-09-23 17:55) - PLID 44916 - ensure that ampersands display properly
		strMenuDescription.Replace("&", "&&");

		menu.AppendMenu(MF_STRING, nCurrentID, strMenuDescription);

		mapIDs[nCurrentID] = pCrumb->m_nPatientID;
	}
	
	nCurrentID++;
	menu.AppendMenu(MF_MENUBREAK|MF_DISABLED, nCurrentID, "Accessed");
	nCurrentID++;
	menu.AppendMenu(MF_SEPARATOR, nCurrentID, "");
	
	pos = m_listBreadcrumbs.GetHeadPosition();
	nCurrentRowPosition = 0;
	while (pos) {
		CBreadcrumb* pCrumb = m_listBreadcrumbs.GetNext(pos);
		nCurrentID++;
		nCurrentRowPosition++;
		
		COleDateTime dtNow = COleDateTime::GetCurrentTime();
		COleDateTime dtDate = pCrumb->m_ftDate;
		COleDateTimeSpan dts = dtDate - dtNow;

		bool bIncludeDate = true;
		if (abs(dts.GetTotalHours()) < 24) {
			bIncludeDate = false;
		}

		CString strWhen = DescribeRelativeDateTimeSpan(dts);
		
		strWhen += " (";
		if (!bIncludeDate) {
			strWhen += FormatDateTimeForInterface(dtDate, 0, dtoTime);
		} else {
			strWhen += FormatDateTimeForInterface(dtDate, 0, dtoDateTime);
		}
		strWhen += ")";

		menu.AppendMenu(MF_STRING, nCurrentID, strWhen);

		mapIDs[nCurrentID] = pCrumb->m_nPatientID;
	}
}

// (a.walling 2010-05-21 10:12) - PLID 17768 - Serialize the data to preferences
void CPatientToolBar::CBreadcrumbTrail::Save()
{
	try {
		DWORD dwSize = 0;
		POSITION pos = m_listBreadcrumbs.GetTailPosition();
		while (pos) {
			if (dwSize == 0) {
				dwSize += sizeof(BYTE); // for version
			}

			CBreadcrumb* pBreadcrumb = m_listBreadcrumbs.GetPrev(pos);

			dwSize += sizeof(pBreadcrumb->m_nPatientID);
			dwSize += sizeof(pBreadcrumb->m_nUserDefinedID);
			dwSize += sizeof(pBreadcrumb->m_ftDate);
			dwSize += (sizeof(pBreadcrumb->m_strDescription.GetLength()));
			dwSize += (pBreadcrumb->m_strDescription.GetLength()); // no null
		}
		pos = NULL;

		if (dwSize > 0) {		
			COleSafeArray sa;
			sa.CreateOneDim(VT_UI1, dwSize);
			
			BYTE* pData = NULL;
			sa.AccessData((LPVOID*)&pData);
			if (!pData) {
				AfxThrowMemoryException();
			}
			BYTE* pPos = pData;

			const BYTE bVersion = 0x1;
			memcpy(pPos, &bVersion, sizeof(BYTE));
			pPos += sizeof(BYTE);
			
			POSITION pos = m_listBreadcrumbs.GetTailPosition();
			while (pos) {
				CBreadcrumb* pBreadcrumb = m_listBreadcrumbs.GetPrev(pos);

				memcpy(pPos, &(pBreadcrumb->m_nPatientID), sizeof(pBreadcrumb->m_nPatientID));
				pPos += sizeof(pBreadcrumb->m_nPatientID);
				memcpy(pPos, &(pBreadcrumb->m_nUserDefinedID), sizeof(pBreadcrumb->m_nUserDefinedID));
				pPos += sizeof(pBreadcrumb->m_nUserDefinedID);
				memcpy(pPos, &(pBreadcrumb->m_ftDate), sizeof(pBreadcrumb->m_ftDate));
				pPos += sizeof(pBreadcrumb->m_ftDate);

				int nLength = pBreadcrumb->m_strDescription.GetLength();
				memcpy(pPos, &(nLength), sizeof(nLength));
				pPos += sizeof(nLength);

				memcpy(pPos, (LPCSTR)pBreadcrumb->m_strDescription, pBreadcrumb->m_strDescription.GetLength());
				pPos += (pBreadcrumb->m_strDescription.GetLength());
			}
			
			sa.UnaccessData();
			pData = NULL;
			
			
			// (a.walling 2010-08-10 13:16) - PLID 40060 - Memory leak. Detach returns a VARIANT which is copied into the _variant_t return value. The original
			// VARIANT is leaked.
			_variant_t varData(sa.Detach(), false);

			SetRemotePropertyImage("PatientBreadcrumbTrailSerialized", varData, 0, GetCurrentUserName());
			SetRemotePropertyImage("PatientBreadcrumbTrailSerialized", varData, 0, GetCurrentUserComputerName());
		}
	} NxCatchAll_NoParent("CBreadcrumbTrail::Save"); // (a.walling 2014-05-05 13:32) - PLID 61945
}

// (a.walling 2010-05-21 10:12) - PLID 17768 - Load from preferences
void CPatientToolBar::CBreadcrumbTrail::Load()
{
	try {
		g_propManager.CachePropertiesInBulk("PATIENTTOOLBAR-BreadcrumbPersist", propImage,
			"(Username = '%s' OR Username = '%s') AND ("
			"Name = 'PatientBreadcrumbTrailSerialized' "
			")", _Q(GetCurrentUserName()), _Q(GetCurrentUserComputerName()));

		Clear();

		LoadFromVariant(GetRemotePropertyImage("PatientBreadcrumbTrailSerialized", 0, GetCurrentUserName(), true));
		LoadFromVariant(GetRemotePropertyImage("PatientBreadcrumbTrailSerialized", 0, GetCurrentUserComputerName(), true));		
	} NxCatchAll_NoParent("CBreadcrumbTrail::Load");
}

void CPatientToolBar::CBreadcrumbTrail::LoadFromVariant(_variant_t& varData)
{
	if (varData.vt == (VT_UI1 | VT_ARRAY)) {
		COleSafeArray sa;
		sa.Attach(varData);

		DWORD dwSize = sa.GetOneDimSize();

		BYTE *pData = NULL;
		sa.AccessData((LPVOID *)&pData);
		if (!pData) {
			AfxThrowMemoryException();
		}
		BYTE* pPos = pData;
		BYTE* pMax = pData + dwSize;

		BYTE bVersion = 0x0;
		memcpy(&bVersion, pPos, sizeof(BYTE));
		pPos += sizeof(BYTE);

		if (bVersion == 0x1) {
			int nCount = 0;
			while (pPos < pMax) {				
				long nPatientID = -1;
				long nUserDefinedID = -1;
				FILETIME ftDate;
				::ZeroMemory(&ftDate, sizeof(ftDate));
				CString strDescription;

				memcpy(&nPatientID, pPos, sizeof(nPatientID));
				pPos += sizeof(nPatientID);
				memcpy(&(nUserDefinedID), pPos, sizeof(nUserDefinedID));
				pPos += sizeof(nUserDefinedID);

				memcpy(&ftDate, pPos, sizeof(ftDate));
				pPos += sizeof(ftDate);

				int nLength = 0;
				memcpy(&(nLength), pPos, sizeof(nLength));
				pPos += sizeof(nLength);

				LPSTR szDescription = strDescription.GetBufferSetLength(nLength + 1);
				::ZeroMemory(szDescription, nLength + 1);
				memcpy(szDescription, pPos, nLength);
				strDescription.ReleaseBuffer();

				pPos += nLength;
				
				AddFromData(nPatientID, nUserDefinedID, ftDate, strDescription);
				nCount++;
			}
		} else {
			AfxThrowInvalidArgException();
		}
		
		sa.UnaccessData();
		sa.Clear();
	}
}

// (a.walling 2013-03-01 15:59) - PLID 55398 - Cache recent rows for person IDs in the patient toolbar to avoid massive memory faults

namespace PatientToolbar
{
struct CachedRowByTicks
	: public std::binary_function<const std::pair<long, PatientToolbar::CachedRow>&, const std::pair<long, PatientToolbar::CachedRow>&, bool>
{
	CachedRowByTicks(DWORD baseTickCount)
		: baseTickCount(baseTickCount)
	{}

	DWORD baseTickCount;

	bool operator()(const std::pair<long, PatientToolbar::CachedRow>& l, const std::pair<long, PatientToolbar::CachedRow>& r) const {
		return (baseTickCount - l.second.ticks) < (baseTickCount - r.second.ticks); // handle tick overflow
	}
};
}

// (a.walling 2013-03-01 15:59) - PLID 55398 - Check cache, then scan
// (a.walling 2015-02-19 08:56) - PLID 64653 - Do not FindByColumn if requerying if bWaitForRequery is false
NXDATALIST2Lib::IRowSettingsPtr CPatientToolBar::FindRowByPersonID(long nID, bool bWaitForRequery)
{
	PatientToolbar::RowCache::iterator it = m_rowCache.find(nID);
	if (it != m_rowCache.end()) {
		it->second.ticks = ::GetTickCount();
		return it->second.pRow;
	}

	NXDATALIST2Lib::IRowSettingsPtr pRow = m_toolBarCombo->CurSel;
	if (pRow && (nID != (long)pRow->Value[0])) {
		pRow = NULL;
	}

	// (a.walling 2015-02-19 08:56) - PLID 64653 - Do not FindByColumn if requerying since it will block until column is found or requery is done
	if (!pRow && !m_toolBarCombo->IsRequerying() && bWaitForRequery) {
		pRow = m_toolBarCombo->FindByColumn(0, nID, NULL, FALSE);
	}

	if (pRow) {
		// (a.walling 2014-04-24 12:00) - VS2013 - emplace rather than insert
		m_rowCache.emplace(nID, pRow);

		// let's not exceed 64 entries
		static const size_t maxSize = 64;
		if (m_rowCache.size() > maxSize) {
			std::vector<std::pair<long, PatientToolbar::CachedRow>> newCache(m_rowCache.begin(), m_rowCache.end());
			sort(newCache.begin(), newCache.end(), PatientToolbar::CachedRowByTicks(::GetTickCount()));

			// remove the oldest 32 items, so we don't have to sort until 32 more new entries have been inserted
			m_rowCache = PatientToolbar::RowCache(newCache.begin() + (maxSize / 2), newCache.end());
		}
	}

	return pRow;
}

void CPatientToolBar::OnDestroy()
{
	try {
		m_rowCache.clear();
	} NxCatchAll(__FUNCTION__);

	__super::OnDestroy();
}

//TES 8/14/2014 - PLID 63520 - Note that DoesPatientExistInList() checks the data, to see if the patient SHOULD be in the list. 
// This function only checks the datalist itself.
BOOL CPatientToolBar::IsPersonCurrentlyInList(long nPersonID)
{
	return FindRowByPersonID(nPersonID) != NULL;
}