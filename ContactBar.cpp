// ContactBar.cpp : implementation file
//

#include "stdafx.h"
#include "practice.h"
#include "ContactBar.h"
#include "PracProps.h"
#include "NxStandard.h"
#include "MainFrm.h"
#include "NxTabView.h"
#include "ChildFrm.h"
#include "NxGdiPlusUtils.h"
#include "PracticeRc.h"
#include "ContactView.h" // (k.messina 2010-04-12 11:15) - PLID 37957

using namespace Gdiplus;

using namespace ADODB;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NXDATALISTLib;
/////////////////////////////////////////////////////////////////////////////
// CContactBar

// (a.walling 2008-08-20 15:43) - PLID 29642 - Enumerate toolbar button indexes
enum EContactToolbarButtons {
	ctbLeft = 1,
	ctbRight = 3,
	ctbAdd = 4,
};

// (a.walling 2008-08-20 15:42) - PLID 29642 - Prepare theme variables
CContactBar::CContactBar()
 : m_uxtTheme(*(new UXTheme))
{
	// (a.walling 2012-01-04 17:13) - PLID 47324 - Disable active accessibility for this window due to module state issues (see this item notes for more info)
	m_bEnableActiveAccessibility = false;

	m_bThemeInit = FALSE;

	m_nLastMouseOverIx = LONG_MIN;
	m_bTrackingMouse = FALSE;

	//m_bEnableActiveAccessibility = false;
}

// (a.walling 2008-08-20 15:42) - PLID 29642 - Cleanup theme data
CContactBar::~CContactBar()
{
	m_searchButtonFont.DeleteObject();
	m_uxtTheme.CloseThemeData();
	delete &m_uxtTheme;
}

// (a.walling 2008-08-20 15:43) - PLID 29642 - Add handlers for CustomDraw, EraseBkgnd, MouseMove, and MouseLeave
// (a.walling 2008-08-21 14:09) - PLID 31056 - Added ON_WM_LBUTTONUP to fix drawing issue with tooltips
// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing (NcPaint)
BEGIN_MESSAGE_MAP(CContactBar, CToolBar)
	//{{AFX_MSG_MAP(CContactBar)
	ON_WM_ENABLE()
	ON_WM_CTLCOLOR()
	ON_WM_ERASEBKGND()
	ON_NOTIFY_REFLECT_EX(NM_CUSTOMDRAW, OnCustomDraw)
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_NCPAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_EVENTSINK_MAP(CContactBar, CToolBar)
	//{{AFX_EVENTSINK_MAP(CContactBar)
	ON_EVENT(CContactBar, IDW_COMBO, 16 /* SelChosen */, OnSelChosenCombo, VTS_I4)
	ON_EVENT(CContactBar, IDW_COMBO, 18 /* RequeryFinished */, OnRequeryFinishedCombo, VTS_I2)
	//}}AFX_EVENTSINK_MAP
END_EVENTSINK_MAP()

/////////////////////////////////////////////////////////////////////////////
// CContactBar message handlers
long CContactBar::GetActiveContact()
{
	if (m_ActiveContact > 0) {
		return m_ActiveContact;
	} else {
		ThrowNxException("CContactBar::GetActiveContact: There is no active contact!");
	}
	/*_variant_t var;
	var = m_toolBarCombo->GetValue(m_toolBarCombo->GetCurSel(),0);
	if(var.vt == VT_NULL || var.vt == VT_EMPTY)
		return 0;
	else
		return var.lVal;*/
}

CString CContactBar::GetActiveContactName()
{
	_variant_t var = m_toolBarCombo->GetValue(m_toolBarCombo->GetCurSel(),5);
	if(var.vt == VT_BSTR)
		return CString(var.bstrVal);
	else
		return "";		
}

long CContactBar::GetActiveContactStatus()
{
	//first try to get the status from the combo, if the person is in the list
	if(m_toolBarCombo) {
		if(m_toolBarCombo->GetCurSel() != -1) {		

			_variant_t var = m_toolBarCombo->GetValue(m_toolBarCombo->GetCurSel(),12);
			if(var.vt == VT_I4)
				return VarLong(var,-1);
		}
	}

	//if we're still here, we need to query the data (can happen on startup
	//because the load is called before the toolbar is complete)

	return GetExistingContactStatus(m_ActiveContact);
}

long CContactBar::GetExistingContactStatus(long nPersonID)
{
	//first try to get the status from the combo, if the person is in the list

	if (m_toolBarCombo) {
		// The toolbar exists
		long nCurSel = m_toolBarCombo->FindByColumn(0,(long)nPersonID,0,FALSE);
		if (nCurSel >= 0) {
			// The person exists so return the status
			_variant_t var = m_toolBarCombo->GetValue(nCurSel,12);
			if(var.vt == VT_I4)
				return VarLong(var,-1);
		}
	}

	//if we're still here, we can't get the status from the contacts list, so get it from data

	long nStatus = -1;

	try {
		_RecordsetPtr rs = CreateRecordset("SELECT PersonT.ID, SupplierT.PersonID AS Sup, ReferringPhyST.PersonID AS Ref, "
			"ProvidersT.PersonID AS Prov, UsersT.PersonID AS UserName, "
			"ContactsT.PersonID AS Contact FROM "
			"PersonT LEFT JOIN SupplierT ON PersonT.ID = SupplierT.PersonID "
			"LEFT JOIN ReferringPhyST ON PersonT.ID = ReferringPhyST.PersonID "
			"LEFT JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID "
			"LEFT JOIN UsersT ON PersonT.ID = UsersT.PersonID "
			"LEFT JOIN ContactsT ON PersonT.ID = ContactsT.PersonID "
			"WHERE PersonT.ID = %li", nPersonID);

		if(!rs->eof) {
			if(rs->Fields->Item["Sup"]->Value.vt != VT_NULL)
				nStatus = 8;
			else if(rs->Fields->Item["Ref"]->Value.vt != VT_NULL)
				nStatus = 1;
			else if(rs->Fields->Item["Prov"]->Value.vt != VT_NULL)
				nStatus = 2;
			else if(rs->Fields->Item["UserName"]->Value.vt != VT_NULL)
				nStatus = 4;
			else if(rs->Fields->Item["Contact"]->Value.vt != VT_NULL)
				nStatus = 0;
		}
		rs->Close();

	} NxCatchAll("Error in CContactBar::GetExistingContactStatus()");

	return nStatus;
}

void CContactBar::SetExistingContactStatus(long nPersonID, long nStatusID)
{
	// (c.haag 2006-10-06 09:27) - PLID 22863 - This function updates the type column
	// of the contacts dropdown for a given contact
	try {
		if (m_toolBarCombo) {
			// The toolbar exists
			long nCurSel = m_toolBarCombo->FindByColumn(0,(long)nPersonID,0,FALSE);
			if (nCurSel >= 0) {
				// The person exists so set the type
				m_toolBarCombo->PutValue(nCurSel, 12, nStatusID);
			}
		}
	}
	NxCatchAll("Error in CContactBar::SetExistingContactType()");
}

CString CContactBar::GetExistingContactName(long PersonID)
{
	if (m_toolBarCombo) {
		// The toolbar exists
		long nCurSel = m_toolBarCombo->FindByColumn(0,(long)PersonID,0,FALSE);
		if (nCurSel >= 0) {
			// The person exisits so return the name
			CString str = VarString(m_toolBarCombo->Value[nCurSel][5]);
			//trim off the " - "
			if(str.Right(3) == " - ")
				str = str.Left(str.GetLength() - 3);
			return str;
		}
	}

	// (j.jones 2010-11-30 16:12) - PLID 31392 - if we are still here we need to query for the name
	_RecordsetPtr rs = CreateParamRecordset("SELECT Last + ', ' + First + ' ' + Middle + ' ' + Title + ' - ' + Company AS Name "
		"FROM PersonT WHERE ID = {INT}", PersonID);
	if(!rs->eof) {
		return AdoFldString(rs, "Name","");
	}

	return "";
}

// (c.haag 2009-05-07 15:58) - PLID 28561 - This function returns a username given a valid UserID
CString CContactBar::GetExistingUserName(long PersonID)
{
	try {
		if (m_toolBarCombo) {
			// The toolbar exists
			long nCurSel = m_toolBarCombo->FindByColumn(0,(long)PersonID,0,FALSE);
			if (nCurSel >= 0) {
				// The person exists, so return the name if the type is also user
				CString strType = VarString(m_toolBarCombo->Value[nCurSel][11]);
				if (strType == "User") {
					CString str = VarString(m_toolBarCombo->Value[nCurSel][10]);				
					//trim off the brackets
					str.TrimLeft(" <");
					str.TrimRight("> ");
					return str;
				} else {
					// Not a user. Maybe the contact type was changed? Regardless,
					// the person can't have a username. Try again via data; maybe
					// the dropdown is outdated.
				}
			} else {
				// No valid selection
			}
		}

		// Try again via data
		_RecordsetPtr prs = CreateParamRecordset("SELECT UserName FROM UsersT WHERE PersonID = {INT}", PersonID);
		if (!prs->eof) {
			return AdoFldString(prs, "UserName", "");
		}
	}
	NxCatchAll("Error in CContactBar::GetExistingUserName");
	return "";
}

// (a.walling 2007-05-04 09:53) - PLID 4850 - Called when the user has been changed, refresh any user-specific settings
void CContactBar::OnUserChanged()
{
	try {

		m_all.SetCheck(true);
		m_supplier.SetCheck(false);
		m_other.SetCheck(false);
		m_referring.SetCheck(false);
		m_main.SetCheck(false);
		m_employee.SetCheck(false);
		SetComboWhereClause("");

		// (j.jones 2007-08-09 16:35) - PLID 27036 - added ability to keep the current patient/contact when switching users
		long nSwitchUser_KeepCurrentPatientContact = GetRemotePropertyInt("SwitchUser_KeepCurrentPatientContact", 0, 0, GetCurrentUserName(), true);
		//0 - keep the current contact, 1 - load the new user's last contact
		if(nSwitchUser_KeepCurrentPatientContact == 1) {			
			long nContactID = GetRemotePropertyInt("lastContact", -1, 0, GetCurrentUserName(), true);
			m_toolBarCombo->SetSelByColumn(0, nContactID);
			SetActiveContactID(nContactID);
		}

	} NxCatchAllThrow("Error initializing contact toolbar");
}

BOOL CContactBar::CreateComboBox()
{
	try {

	CRect	rect,
			trueRect,
			comboRect;
	int		rectHeight;

	const int width1 = 65;
	const int width2 = 82;
	const int width3 = 70;
	const int width4 = 75;
	const int width5 = 95;
		
	const int height = 20;

	// (c.haag 2009-08-05 12:50) - PLID 20750 - Bulk caching
	g_propManager.CachePropertiesInBulk("CContactBar_CreateComboBox", propNumber,
		"(Username = '<None>' OR Username = '%s') AND ("
		"Name = 'HideInactiveContacts' OR "
		"Name = 'lastContact' "
		")",
		_Q(GetCurrentUserName()));

	//TES 4/1/2008 - PLID 29485 - Need to use a function that gives the same results in VS6 and VS 2008.
	// (a.walling 2008-11-13 10:46) - PLID 31956 - Now that NxButton is fixed, we can revert back to DEFAULT_QUALITY
	// (a.walling 2016-06-01 11:12) - NX-100195 - use Segoe UI
	CreateCompatiblePointFont(&m_searchButtonFont, 100, "Segoe UI", NULL, DEFAULT_QUALITY);

	SetHeight(TOOLBARHEIGHT);

	//Create Buttons
	SetButtonInfo(0, IDW_COMBO, TBBS_SEPARATOR, width1 + width2 + width3 + width4 + width5);

	GetItemRect(0, &trueRect);
	rect = trueRect;
	rectHeight = rect.bottom - rect.top;
	rect.bottom = rect.top + height;
	rect.left;
	rect.right = rect.left + width1;

	if(m_searchText.Create("  Include:", WS_CHILD|WS_GROUP|WS_VISIBLE|SS_LEFT, rect, this, IDC_TEXT) == 0)
		return FALSE;
	m_searchText.SetFont(&m_searchButtonFont, FALSE);

	rect.left = rect.right;
	rect.right += width2;
	if (m_main.Create("Provider", WS_TABSTOP|WS_VISIBLE|BS_AUTORADIOBUTTON, rect, this, IDC_MAIN_SEARCH) == 0)
		return FALSE;
	m_main.SetFont(&m_searchButtonFont, FALSE);
	m_main.SetCheck(false);

	rect.top += height;
	rect.bottom += height;
	if (m_referring.Create("Ref. Phys.", WS_TABSTOP|WS_VISIBLE|BS_AUTORADIOBUTTON, rect, this, IDC_REFERING_SEARCH) == 0)
		return FALSE;
	m_referring.SetFont(&m_searchButtonFont, FALSE);
	m_referring.SetCheck(false);

	rect.left = rect.right;
	rect.right += width3;
	if (m_employee.Create("Users", WS_TABSTOP|WS_VISIBLE|BS_AUTORADIOBUTTON, rect, this, IDC_EMPLOYEE_SEARCH) == 0)
		return FALSE;
	m_employee.SetFont(&m_searchButtonFont, FALSE);
	m_employee.SetCheck(false);

	rect.bottom -= height;
	rect.top -= height;
	if (m_other.Create("Other", WS_TABSTOP|WS_VISIBLE|BS_AUTORADIOBUTTON, rect, this, IDC_OTHER_SEARCH) == 0)
		return FALSE;
	m_other.SetFont(&m_searchButtonFont, FALSE);
	m_other.SetCheck(false);

	rect.left = rect.right;
	rect.right += width4;
	if (m_supplier.Create("Supplier", WS_TABSTOP|WS_VISIBLE|BS_AUTORADIOBUTTON, rect, this, IDC_SUPPLIER_SEARCH) == 0)
		return FALSE;
	m_supplier.SetFont(&m_searchButtonFont, FALSE);
	m_supplier.SetCheck(false);

	rect.bottom += height;
	rect.top += height;
	if (m_all.Create("All", WS_TABSTOP|WS_VISIBLE|BS_AUTORADIOBUTTON, rect, this, IDC_CONTACTS_SEARCH) == 0)
		return FALSE;
	m_all.SetFont(&m_searchButtonFont, FALSE);
	m_all.SetCheck(true);

	rect.left = rect.right;
	rect.right += width5;
	if (m_hideInactiveContacts.Create("Hide Inactive", WS_GROUP|WS_TABSTOP|WS_VISIBLE|BS_AUTOCHECKBOX, rect, this, IDC_ACTIVE_CONTACTS) == 0)
		return FALSE;
	m_hideInactiveContacts.SetFont(&m_searchButtonFont, FALSE);
	// (c.haag 2009-08-05 12:49) - PLID 20750 - Recall this selection for this user
	m_hideInactiveContacts.SetCheck( GetRemotePropertyInt("HideInactiveContacts", 0, 0, GetCurrentUserName(), TRUE) ? 1 : 0);
	
	//	SetButtonInfo(1, IDW_COMBO, TBBS_SEPARATOR, 0);//75
//	SetButtonInfo(2, IDW_COMBO, TBBS_SEPARATOR, 0);//88
	//SetButtonInfo(3, IDW_COMBO, TBBS_SEPARATOR, 0);//40
	//Create Combo
#define COMBO_BUTTON 2
	int nDesktopWidth = GetSystemMetrics(SM_CXSCREEN);
	int nDatalistWidth;
	if(nDesktopWidth <= 800) //Make sure the new contact button will still be visible
		nDatalistWidth = 290;
	else
		nDatalistWidth = 365;
	SetButtonInfo(COMBO_BUTTON, IDW_COMBO, TBBS_SEPARATOR, nDatalistWidth);
	GetItemRect(COMBO_BUTTON, &rect);
	rectHeight = rect.bottom - rect.top;
	rect.bottom = rect.top + 19;

	rect.top -= 1;
	rect.bottom += 8;

	/* TODO: The other toolbars work just fine, I don't know why this one does not but it doesn't so I'm commenting it out
	// Create a static control that covers all the separator buttons to hide them (so we don't see ugly vertical bars behind the combo box)
	m_lblBlank.Create("", WS_CHILD|WS_VISIBLE|WS_GROUP|SS_CENTER, CRect(rect.left, 0, rect.right, 600), this);
	*/

	if (m_wndContactsCombo.CreateControl(_T("NXDATALIST.NxDataListCtrl.1"), "Contacts Bar", WS_CHILD, rect, this, IDW_COMBO)) {

		m_toolBarCombo = m_wndContactsCombo.GetControlUnknown();
		ASSERT(m_toolBarCombo != NULL); // We need to be able to get he control unknown and set an NxDataListPtr to point to it
		if (m_toolBarCombo) {
			try {
				// Set up the columns
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(0, _T("PersonID"), _T("ID"), 00, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;//cftNumberBasic;
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(1, _T("Last"), _T("Last Name"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(2, _T("First"), _T("First Name"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(3, _T("Middle"), _T("Middle"), 8, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(4, _T("Title"), _T("Title"), 16, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(5, _T("Last + ', ' + First + ' ' + Middle + ' ' + Title + ' - ' + Company"), _T("Name"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(6, _T("Refer"), _T("Refer"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(7, _T("Main"), _T("Main"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(8, _T("Employee"), _T("Employee"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(9, _T("Supplier"), _T("Supplier"), 0, csVisible|csFixedWidth)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(10, _T("Company"), _T("Company/Username"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(11, _T("Type"), _T("Type"), -1, csVisible|csWidthAuto)))->FieldType = cftTextSingleLine;
				IColumnSettingsPtr(m_toolBarCombo->GetColumn(m_toolBarCombo->InsertColumn(12, _T("Status"), _T("Status"), 0, csVisible|csWidthPercent)))->FieldType = cftTextSingleLine;

//setup the string based on the permissions of this user				
				CString strSql;

				//DRT 5/2/03 - I removed this setup.  It's horribly ugly, hard to figure out what's going on, and in general, 
				//		just a big mess.  I re-made it below to just be 1 query that pulls everyone, regardless of your permissions.
				//		Then I modified the SetComboWhereClause() function to handle the filtering out of the ones we
				//		shouldn't be able to see.  All in all, much prettier and easier to understand than this mess.
/*
				//DRT 3/10/03 - Changed the company column to show username if they are a user - since the user's company is going to be the 
				//		office in most cases
				strSql = "(SELECT PersonT.ID AS PersonID, PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, "
				"CASE WHEN UsersT.PersonID IS NULL THEN PersonT.Company ELSE '< ' + UsersT.UserName + ' >' END AS Company, ";

				//changed to follow sptRead permission
				if(GetCurrentUserPermissions(bioContactsRefPhys) & sptRead)
					strSql += "(CASE WHEN([ReferringPhyST].[PersonID] Is Null) THEN 0 ELSE 1 END) AS Refer, ";
				else
					strSql += "0 AS Refer, ";

				if(GetCurrentUserPermissions(bioContactsProviders) & SPT___W_______)
					strSql += "(CASE WHEN (ProvidersT.[PersonID] Is Null) THEN 0 ELSE 1 END) AS Main, ";
				else
					strSql += "0 AS Main, ";

				if(GetCurrentUserPermissions(bioContactsUsers) & SPT___W_______)
					strSql += "(CASE WHEN(UsersT.PersonID Is Null) THEN 0 ELSE 1 END) AS Employee, ";
				else
					strSql += "0 AS Employee, ";

				strSql += "(CASE WHEN(SupplierT.PersonID Is Null) THEN 0 ELSE 1 END) AS Supplier FROM PersonT "
				"LEFT JOIN [ContactsT] ON PersonT.ID = ContactsT.PersonID LEFT JOIN SupplierT "
				"ON PersonT.ID = [SupplierT].[PersonID] ";

			//DRT 4/1/03 - I don't see the reasoning for leaving these out of the query, we can put them in the from clause and
			//			still have no problems with the above
				if(GetCurrentUserPermissions(bioContactsRefPhys) & sptRead)
					strSql += "LEFT JOIN ReferringPhyST ON PersonT.ID = ReferringPhyST.PersonID ";
				
			//	if(GetCurrentUserPermissions(bioContactsProviders) & SPT___W_______)
					strSql += "LEFT JOIN [ProvidersT] ON PersonT.ID = [ProvidersT].[PersonID] ";

			//	if(GetCurrentUserPermissions(bioContactsUsers) & SPT___W_______)
					strSql += "LEFT JOIN UsersT ON PersonT.ID = [UsersT].[PersonID] ";
				
				strSql += "WHERE (ContactsT.PersonID Is Not Null OR SupplierT.PersonID Is Not Null ";
				
				if(GetCurrentUserPermissions(bioContactsRefPhys) & sptRead)
					strSql += " OR ReferringPhyST.PersonID Is Not Null ";
				if(GetCurrentUserPermissions(bioContactsProviders) & SPT___W_______)
					strSql += "OR ProvidersT.PersonID Is Not Null ";
				if(GetCurrentUserPermissions(bioContactsUsers) & SPT___W_______)
					strSql += "OR UsersT.PersonID Is Not Null";

				strSql += ")) AS ContactStatusQ";
*/
//end setting up this blasted query

				//DRT 5/2/03 - Make the query select everyone regardless.  Then make a where clause that filters out things
				//		we don't have permission for.
				strSql = "(SELECT PersonT.ID AS PersonID, PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.Archived AS Archived, "
				"CASE WHEN UsersT.PersonID IS NULL THEN PersonT.Company ELSE '< ' + UsersT.UserName + ' >' END AS Company, "
				"(CASE WHEN([ReferringPhyST].[PersonID] Is Null) THEN 0 ELSE 1 END) AS Refer, "
				"(CASE WHEN (ProvidersT.[PersonID] Is Null) THEN 0 ELSE 1 END) AS Main, "
				"(CASE WHEN(UsersT.PersonID Is Null) THEN 0 ELSE 1 END) AS Employee, "

				"(CASE WHEN([ReferringPhyST].[PersonID] Is Not Null) THEN 'Ref. Phys' "
				"ELSE CASE WHEN(ProvidersT.[PersonID] Is Not Null) THEN 'Provider' "
				"ELSE CASE WHEN(UsersT.PersonID Is Not Null) THEN 'User' "
				"ELSE CASE WHEN(SupplierT.PersonID Is Not Null) THEN 'Supplier' "
				"ELSE 'Other' END END END END) AS Type, "

				"(CASE WHEN([ReferringPhyST].[PersonID] Is Not Null) THEN 1 "
				"ELSE CASE WHEN(ProvidersT.[PersonID] Is Not Null) THEN 2 "
				"ELSE CASE WHEN(UsersT.PersonID Is Not Null) THEN 4 "
				"ELSE CASE WHEN(SupplierT.PersonID Is Not Null) THEN 8 "
				"ELSE 0 END END END END) AS Status, "

				"(CASE WHEN(SupplierT.PersonID Is Null) THEN 0 ELSE 1 END) AS Supplier FROM PersonT "
				"LEFT JOIN [ContactsT] ON PersonT.ID = ContactsT.PersonID LEFT JOIN SupplierT "
				"ON PersonT.ID = [SupplierT].[PersonID] "
				"LEFT JOIN ReferringPhyST ON PersonT.ID = ReferringPhyST.PersonID "
				"LEFT JOIN [ProvidersT] ON PersonT.ID = [ProvidersT].[PersonID] "
				"LEFT JOIN UsersT ON PersonT.ID = [UsersT].[PersonID] AND UsersT.PersonID > 0 "
				"WHERE (ContactsT.PersonID Is Not Null OR SupplierT.PersonID Is Not Null "
				" OR ReferringPhyST.PersonID Is Not Null "
				"OR ProvidersT.PersonID Is Not Null "
				"OR UsersT.PersonID Is Not Null"
				")) AS ContactStatusQ";

				m_toolBarCombo->FromClause = _T(_bstr_t(strSql));

				//	Non-permission from clause
				//	m_toolBarCombo->FromClause = _T("(SELECT PersonT.ID AS PersonID, PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.Company, (CASE WHEN([ReferringPhyST].[PersonID] Is Null) THEN 0 ELSE 1 END) AS Refer, (CASE WHEN (ProvidersT.[PersonID] Is Null) THEN 0 ELSE 1 END) AS Main, (CASE WHEN(UsersT.PersonID Is Null) THEN 0 ELSE 1 END) AS Employee, (CASE WHEN(SupplierT.PersonID Is Null) THEN 0 ELSE 1 END) AS Supplier FROM PersonT LEFT JOIN [ContactsT] ON PersonT.ID = ContactsT.PersonID LEFT JOIN SupplierT ON PersonT.ID = [SupplierT].[PersonID]  LEFT JOIN ReferringPhyST ON PersonT.ID = ReferringPhyST.PersonID LEFT JOIN [ProvidersT] ON PersonT.ID = [ProvidersT].[PersonID] LEFT JOIN UsersT ON PersonT.ID = [UsersT].[PersonID] WHERE (ContactsT.PersonID Is Not Null OR SupplierT.PersonID Is Not Null OR ReferringPhyST.PersonID Is Not Null OR ProvidersT.PersonID Is Not Null OR UsersT.PersonID Is Not Null)) AS ContactStatusQ");

				// We want it to be a combo, not a datalist
				m_toolBarCombo->IsComboBox = TRUE;
				m_toolBarCombo->DisplayColumn = "[1], [2] [3] [4] - [10]";
				m_toolBarCombo->TextSearchCol = 1;
				m_toolBarCombo->GetColumn(1)->PutSortPriority(0);
				m_toolBarCombo->GetColumn(1)->PutSortAscending(TRUE);
				m_toolBarCombo->GetColumn(2)->PutSortPriority(1);
				m_toolBarCombo->GetColumn(2)->PutSortAscending(TRUE);
				m_toolBarCombo->DropDownWidth = 450;
				m_toolBarCombo->AutoDropDown = TRUE;
				// (a.walling 2010-07-28 14:10) - PLID 39871 - Choose snapshot isolation connection if preferred
				// (a.walling 2012-02-09 17:13) - PLID 45448 - NxAdo unification - GetRemoteDataOrDataSnapshot -> GetRemoteDataSnapshot
				m_toolBarCombo->AdoConnection = GetRemoteDataSnapshot();
				m_ActiveContact = GetRemotePropertyInt("lastContact", -1, 0, GetCurrentUserName(), true);

				// (b.cardillo 2009-02-18 16:26) - PLID 33155 - Moved this SetComboWhereClause() call to AFTER 
				// the datalist columns and other properties have been set (most notably the connection), and 
				// also made it skip our own Requery() if the SetComboWhereClause() requeried.
				//this function handles setting up the permissions for which things to filter out
				if (!SetComboWhereClause("")) {
					m_toolBarCombo->Requery();
				}

				if (m_ActiveContact == -1) {
					try {
						m_toolBarCombo->CurSel = 0;
						// (b.cardillo 2011-08-19 12:33) - PLID 40044 - Look at the actual selected row rather than 
						// the 0th row (which may have shifted since the last line because of incoming rows).
						long curSelEnum = m_toolBarCombo->GetFirstSelEnum();
						if (curSelEnum != NULL) {
							m_ActiveContact = VarLong(m_toolBarCombo->GetValueByEnum(curSelEnum, 0));
						}
					} catch (COleDispatchException *e) {
						if (e->m_wCode == 0xf108) {
							// No contacts which we don't care about
							e->Delete();
						} else {
							throw e;
						}
					} catch (CException *e) {
						throw e;
					}
				} else {
					if(m_toolBarCombo->TrySetSelByColumn(0,(long)m_ActiveContact)==-1) {
						m_toolBarCombo->CurSel = 0;
						if(m_toolBarCombo->CurSel!=-1)
							m_ActiveContact = m_toolBarCombo->GetValue(0,0).lVal;
						else
							m_ActiveContact = -1;
					}
				}

			} NxCatchAll("Error in creating contact bar!");
			// We have success so display the window (TODO: depending on how you're using this, you 
			// might want to display the window first, and not destroy it above on failure)
			m_wndContactsCombo.ShowWindow(SW_SHOW);

			/*TODO: Do we need this?
			m_wndContactsCombo.SetFont(m_Font.m_lpDispatch);
			*/

			/*TODO: Do we need this?
			*/// YES, we do!  This was commented out and that's why the 
			  // dropdown was popping up at the top-left of the screen...
			//center combo box edit control vertically within tool bar
			m_wndContactsCombo.GetWindowRect(&comboRect);
			m_wndContactsCombo.ScreenToClient(&comboRect);  
			m_wndContactsCombo.SetWindowPos(&wndTopMost, rect.left, 
				rect.top + (rectHeight - comboRect.Height())/2+1, 0, 0, 
				SWP_NOZORDER|SWP_NOSIZE|SWP_NOACTIVATE);
			//*/

			// Done, return success
			return TRUE;
		} else {
			// Failed because we couldn't set dlp to the control unknown
			MsgBox("Could not connect to the contacts toolbar.");
			m_wndContactsCombo.DestroyWindow();
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

	}
	NxCatchAll("Error in CContactBar::CreateComboBox");
	return TRUE;
}

void CContactBar::OnRequeryFinishedCombo(short nFlags)
{

	if(nFlags == 0)	//the requery was finished, not interrupted
	{
		//ensure that something was selected
		if(m_toolBarCombo->GetCurSel() != -1)
			return;

		//set the current selection to the first contact
		m_toolBarCombo->CurSel = 0;
		if(m_toolBarCombo->GetCurSel() != -1)
			m_ActiveContact = m_toolBarCombo->GetValue(m_toolBarCombo->GetCurSel(),0).lVal;
		if(GetMainFrame() && GetMainFrame()->GetActiveView() &&
			GetMainFrame()->IsContactsBarVisible())
			GetMainFrame()->GetActiveView()->UpdateView();
	}
}

BOOL CContactBar::PreTranslateMessage(MSG* pMsg) 
{

	// Note: there is only one control in the toolbar that accepts keyboard input,
	// this is the combobox.

	
/*		DRT - 8/20/01  - Chris thinks this is all old code for his combo box and just causes problems with key
						processing now.
	// user hit ENTER
	if (pMsg->wParam == VK_RETURN && GetKeyState(VK_RETURN) < 0)
	{
		// extract the text, update combobox lists, and do the search
		CString s1;
		s1 = CString(m_toolBarCombo->GetValue(m_toolBarCombo->GetCurSel(),1).bstrVal);

		if(s1.GetLength()) {
			return TRUE;  // key processed
		} else {
			MessageBeep(MB_ICONEXCLAMATION);
			return TRUE;  // key processed
		}
	}

	// user hit ESC
	if (pMsg->wParam == VK_ESCAPE && GetKeyState(VK_ESCAPE) < 0)
	{
		return TRUE; // key processed
	}

	// user hit DOWN-ARROW
	if (pMsg->wParam == VK_DOWN && GetKeyState(VK_DOWN) < 0)
	{
		//return TRUE; // key processed
	}
*/
	return CToolBar::PreTranslateMessage(pMsg);
}

void CContactBar::OnEnable(BOOL bEnable) 
{
	CToolBar::OnEnable(bEnable);
}

//These refresh the modules
//TES 5/27/2004: Changed this from SelChanged to SelChosen.
void CContactBar::OnSelChosenCombo(long nRow)
{
	if(nRow == -1) {
		m_toolBarCombo->CurSel = 0;
		m_ActiveContact = VarLong(m_toolBarCombo->GetValue(0,0));
	}
	else {
		m_ActiveContact = VarLong(m_toolBarCombo->GetValue(nRow,0));
	}
	OnPullUpCombo();

	// (k.messina 2010-04-12 11:16) - PLID 37957 lock internal contact notes
	if(IsNexTechInternal()) {
		((CContactView*)GetMainFrame()->GetOpenView("Contacts"))->CheckViewNotesPermissions();
	}
}

void CContactBar::OnPullUpCombo()
{
//	CNxTabView *ptab = (CNxTabView *)GetOpenView(CONTACT_MODULE_NAME);
//	if (ptab)
//	{	ptab->GetActiveSheet()->StoreDetails();
//		ptab->GetActiveSheet()->RecallDetails();
//	}
	if (GetMainFrame() && GetMainFrame()->GetActiveView())
		GetMainFrame()->GetActiveView()->UpdateView();
}

bool CContactBar::SetComboWhereClause(const CString &strNewSQL, bool bForce /* = false */)
{
	// Get the currently selected item, if there is one
	COleVariant place;
	{
		long nCurSel = m_toolBarCombo->GetCurSel();
		if (nCurSel != sriNoRow) {
			place = m_toolBarCombo->GetValue(nCurSel, 0);
		}
	}

	//(e.lally 2005-11-02) PLID 17444 - add ability to make referring physicians inactive
	//(e.lally 2005-11-04) PLID 18152 - add ability to make suppliers inactive
	//(e.lally 2005-11-28) PLID 18153 -	add ability to make other contacts inactive
	if(m_main.GetCheck()		|| m_employee.GetCheck() || 
	   m_referring.GetCheck()	|| m_supplier.GetCheck() ||
	   m_other.GetCheck()		|| m_all.GetCheck()){
		m_hideInactiveContacts.ShowWindow(SW_SHOW);
	}
	else{
		m_hideInactiveContacts.ShowWindow(SW_HIDE);
	}

	CString strSQL = strNewSQL;
	strSQL.TrimRight(';');

	//DRT 5/2/03 - Check permissions and filter out the ones we shouldn't be seeing.
	if(!(GetCurrentUserPermissions(bioContactsSuppliers) & sptRead))
		strSQL += " AND Supplier = 0";

	if(!(GetCurrentUserPermissions(bioContactsRefPhys) & sptRead))
		strSQL += " AND Refer = 0";

	if(!(GetCurrentUserPermissions(bioContactsProviders) & sptRead))
		strSQL += " AND Main = 0";

	if(!(GetCurrentUserPermissions(bioContactsUsers) & sptRead))
		strSQL += " AND Employee = 0";

	
	// See if we want only active patients
	if (m_hideInactiveContacts.GetCheck() == 1) {
		// Add " WHERE (Archived = false)"
		strSQL += " AND Archived = 0";
	}
	
	if(strSQL.Left(5) == " AND "){
		strSQL.Delete(0, 5);
	}
	//strSQL.TrimLeft(" AND ");	//in case we got nothing passed in and added to it (ex: ' AND Refer = 0')

	// (a.walling 2007-11-05 13:07) - PLID 27977 - VS2008 - Operator != is ambiguous

	if (bForce || (strSQL.Compare((LPCTSTR)m_toolBarCombo->WhereClause) != 0) ) 
	{	TRACE("CContactToolBar::SetComboSQL <==> Changing Contact Combo SQL Statement!\n");
		m_toolBarCombo->WhereClause = _bstr_t(strSQL);
		m_toolBarCombo->Requery();
		// Re-select the one that was selected before, if there was one selected before
		if (place.vt != VT_EMPTY && m_toolBarCombo->SetSelByColumn(0,place)==-1) {
			try {
				m_toolBarCombo->CurSel = 0;
			} catch (COleDispatchException *e) {
				if (e->m_wCode == 0xf108) {
					// No contacts which we don't care about
					e->Delete();
				} else {
					throw e;
				}
			} catch (CException *e) {
				throw e;
			}
		}
		if(m_toolBarCombo->GetCurSel() != -1)
			m_ActiveContact = m_toolBarCombo->GetValue(m_toolBarCombo->GetCurSel(),0).lVal;
		return true;
	} 
	else return false;
}

void CContactBar::SetActiveContactID(long nNewContactID)
{
	try {
		if(m_toolBarCombo->SetSelByColumn(0, nNewContactID) != -1) {
			m_ActiveContact = m_toolBarCombo->GetValue(m_toolBarCombo->GetCurSel(),0).lVal;
		}
		else {
			m_all.SetCheck(true);
			m_supplier.SetCheck(false);
			m_other.SetCheck(false);
			m_referring.SetCheck(false);
			m_main.SetCheck(false);
			m_employee.SetCheck(false);
			SetComboWhereClause("");
			m_toolBarCombo->SetSelByColumn(0, nNewContactID);
			if (m_toolBarCombo->GetCurSel() != -1) // (a.walling 2007-06-01 16:51) - PLID 4850 - Don't try to GetValue of a sriNoRow index.
				m_ActiveContact = m_toolBarCombo->GetValue(m_toolBarCombo->GetCurSel(),0).lVal;

			CNxTabView *ptab = (CNxTabView *)GetMainFrame()->GetOpenView(CONTACT_MODULE_NAME);
			if (ptab)
			{	ptab->UpdateView();
			}
		}
	} NxCatchAllThrow("Error setting active contact ID!");
}

void CContactBar::ChangeContact(long nID)
{
	try {
		CString strWhere = (LPCTSTR)m_toolBarCombo->WhereClause;
		CString strSql;
		_RecordsetPtr prs = CreateRecordset("SELECT * FROM (SELECT PersonT.ID AS PersonID, PersonT.Last, PersonT.First, PersonT.Middle, PersonT.Title, PersonT.Archived AS Archived, "
				"CASE WHEN UsersT.PersonID IS NULL THEN PersonT.Company ELSE '< ' + UsersT.UserName + ' >' END AS Company, "
				"Last + ', ' + First + ' ' + Middle + ' ' + Title + ' - ' + Company AS Name, "
				"(CASE WHEN([ReferringPhyST].[PersonID] Is Null) THEN 0 ELSE 1 END) AS Refer, "
				"(CASE WHEN (ProvidersT.[PersonID] Is Null) THEN 0 ELSE 1 END) AS Main, "
				"(CASE WHEN(UsersT.PersonID Is Null) THEN 0 ELSE 1 END) AS Employee, "

				"(CASE WHEN([ReferringPhyST].[PersonID] Is Not Null) THEN 'Ref. Phys' "
				"ELSE CASE WHEN(ProvidersT.[PersonID] Is Not Null) THEN 'Provider' "
				"ELSE CASE WHEN(UsersT.PersonID Is Not Null) THEN 'User' "
				"ELSE CASE WHEN(SupplierT.PersonID Is Not Null) THEN 'Supplier' "
				"ELSE 'Other' END END END END) AS Type, "

				"(CASE WHEN([ReferringPhyST].[PersonID] Is Not Null) THEN 1 "
				"ELSE CASE WHEN(ProvidersT.[PersonID] Is Not Null) THEN 2 "
				"ELSE CASE WHEN(UsersT.PersonID Is Not Null) THEN 4 "
				"ELSE CASE WHEN(SupplierT.PersonID Is Not Null) THEN 8 "
				"ELSE 0 END END END END) AS Status, "

				"(CASE WHEN(SupplierT.PersonID Is Null) THEN 0 ELSE 1 END) AS Supplier FROM PersonT "
				"LEFT JOIN [ContactsT] ON PersonT.ID = ContactsT.PersonID LEFT JOIN SupplierT "
				"ON PersonT.ID = [SupplierT].[PersonID] "
				"LEFT JOIN ReferringPhyST ON PersonT.ID = ReferringPhyST.PersonID "
				"LEFT JOIN [ProvidersT] ON PersonT.ID = [ProvidersT].[PersonID] "
				"LEFT JOIN UsersT ON PersonT.ID = [UsersT].[PersonID]) ContactSubQ "
				"WHERE (PersonID = %d %s)", nID, strWhere.IsEmpty() ? "" : CString("AND ") + strWhere);

		int nRow = m_toolBarCombo->FindByColumn(0, nID, 0, 0);
		if (prs->eof)
		{ // must be a deleted contact
			if (nRow != -1) { //don't remove if not in list

				//if it is the currently selected patient, warn the user
				BOOL bCurrent = FALSE;
				if(m_toolBarCombo->GetCurSel() == nRow) {
					bCurrent = TRUE;

					CChildFrame *p = GetMainFrame()->GetActiveViewFrame();
					if (p) {
						CNxTabView *pView = (CNxTabView *)p->GetActiveView();
						CChildFrame *pFrame = GetMainFrame()->GetActiveViewFrame();
						if (pView && pFrame->IsOfType(CONTACT_MODULE_NAME)) {
							MsgBox("This contact has been deleted by another user.\n"
							"The selection will now be changed to the next contact.");
						}
					}
				}

				//remove the row
				m_toolBarCombo->RemoveRow(nRow);

				if(bCurrent) {					
					OnSelChosenCombo(m_toolBarCombo->GetCurSel());
				}
			}
		}
		else
		{
			IRowSettingsPtr pRow;
			FieldsPtr f = prs->Fields;

			pRow = m_toolBarCombo->GetRow(nRow);

			pRow->Value[0] = f->Item["PersonID"]->Value;
			pRow->Value[1] = f->Item["Last"]->Value;
			pRow->Value[2] = f->Item["First"]->Value;
			pRow->Value[3] = f->Item["Middle"]->Value;
			pRow->Value[4] = f->Item["Title"]->Value;
			pRow->Value[5] = f->Item["Name"]->Value;
			pRow->Value[6] = f->Item["Refer"]->Value;
			pRow->Value[7] = f->Item["Main"]->Value;
			pRow->Value[8] = f->Item["Employee"]->Value;
			pRow->Value[9] = f->Item["Supplier"]->Value;
			pRow->Value[10] = f->Item["Company"]->Value;
			pRow->Value[11] = f->Item["Type"]->Value;
			pRow->Value[12] = f->Item["Status"]->Value;

			//TES 10/11/2005 - This might be a new contact!
			if(nRow == -1) {
				m_toolBarCombo->AddRow(pRow);
			}
		}
	}
	NxCatchAll("Error in CContactBar::ChangeContact");
	
}

void CContactBar::Requery()
{
	// get the current selection so after we requery we can select the same person if they exist
	m_toolBarCombo->Requery();
	m_toolBarCombo->TrySetSelByColumn(0, m_ActiveContact);
}

CString CContactBar::GetComboWhereClause()
{
	return VarString(m_toolBarCombo->WhereClause);
}

void CContactBar::OnTrySetSelFinishedContactCombo(long nRowEnum, long nFlags) 
{
	if(nRowEnum == -1) {
		ChangeContact(m_ActiveContact);
	}
}

// (a.walling 2007-11-12 10:40) - PLID 28062 - Added CtlColor handler
HBRUSH CContactBar::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	// (a.walling 2007-11-27 16:49) - PLID 28062 - VS2008 - Use a solid brush for VC6, transparent for VS2008
	// (a.walling 2008-04-14 11:18) - PLID 29642 - No longer necessary due to overriden DockBar (NxDockBar)
	pDC->SetBkMode(TRANSPARENT);

	return (HBRUSH)::GetStockObject(HOLLOW_BRUSH);
}

// (a.walling 2008-08-20 15:48) - PLID 29642 - We need to custom draw since by removing the FLAT style
// to get rid of separators also gets rid of transparent buttons, leading to ugly raised buttons. Ew.
// So custom draw is our only option to draw correctly now. Unfortunately you cannot custom draw
// separators, or this would have been much simpler.
BOOL CContactBar::OnCustomDraw(NMHDR* pNotifyStruct, LRESULT* result)
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
	
		// adjust the icon a bit too
		rcIcon.DeflateRect(2, 2, 2, 2);

		CRect rcAdjust = rcIcon;
		rcAdjust.DeflateRect(1, 1, 1, 1);

		// which toolbar icon is this?
		WORD nID = 0;

		switch(pDraw->nmcd.dwItemSpec) {
			// main toolbar
			case ID_PREV_CONTACT_BTN: {
				nID = IDI_LARROW;
				pBitmap = NxGdi::BitmapFromPNGResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_PNG_TBARROWLEFT), 24);
				break;
			}
			case ID_NEXT_CONTACT_BTN: {
				nID = IDI_RARROW;
				pBitmap = NxGdi::BitmapFromPNGResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_PNG_TBARROWRIGHT), 24);
				break;
			}
			case ID_NEW_CONTACT: {
				nID = IDI_PLUS;
				pBitmap = NxGdi::BitmapFromPNGResource(AfxGetApp()->m_hInstance, MAKEINTRESOURCE(IDR_PNG_TBADDCONTACT), 24);
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
			} else if ((m_nLastMouseOverIx == ctbLeft && pDraw->nmcd.dwItemSpec == ID_PREV_CONTACT_BTN) || (m_nLastMouseOverIx == ctbRight && pDraw->nmcd.dwItemSpec == ID_NEXT_CONTACT_BTN) || (m_nLastMouseOverIx == ctbAdd && pDraw->nmcd.dwItemSpec == ID_NEW_CONTACT)) {
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
			Graphics gScreen(pDraw->nmcd.hdc);
			gScreen.DrawImage(&bmpBuffer, rcScreen.left, rcScreen.top);
			
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
BOOL CContactBar::OnEraseBkgnd(CDC* pDC)
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
void CContactBar::OnMouseMove(UINT nFlags, CPoint point)
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

			if (nIndex == ctbLeft || nIndex == ctbRight || nIndex == ctbAdd) {
				GetItemRect(nIndex, rcNew);
			}

			if (m_nLastMouseOverIx == ctbLeft || m_nLastMouseOverIx == ctbRight || m_nLastMouseOverIx == ctbAdd) {
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

LRESULT CContactBar::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	if (m_bTrackingMouse && (m_nLastMouseOverIx == ctbLeft || m_nLastMouseOverIx == ctbRight || m_nLastMouseOverIx == ctbAdd)) {
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

void CContactBar::OnLButtonUp(UINT nFlags, CPoint point)
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
	} NxCatchAllCallIgnore({LogDetail("Error handling contact toolbar OnLButtonUp");};);

	CToolBar::OnLButtonUp(nFlags, point);
}

// (a.walling 2008-10-02 17:23) - PLID 31575 - Border drawing (NcPaint)
void CContactBar::OnNcPaint()
{
	// (a.walling 2012-03-02 10:43) - PLID 48589 - No more toolbar borders
	/*
	if (g_bClassicToolbarBorders) {
		CToolBar::OnNcPaint();
	}
	*/
	return;
}