// PharmacyDirectorySeachDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "PharmacyDirectorySearchDlg.h"
#include "InternationalUtils.h"
#include "AdministratorRc.h"
#include "GlobalAuditUtils.h"
#include "SureScriptsPractice.h"
#include "AuditTrail.h"
#include "NxAPI.h"
#include <set>

// (a.walling 2009-10-13 10:01) - PLID 35930
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// (a.walling 2010-01-21 16:43) - PLID 37025 - Modified all auditing to take in a patient's internal ID when applicable, -1 if not.



// (a.walling 2009-03-30 11:31) - PLID 33573 - Interface for SureScripts directories / pharmacy search


// CPharmacyDirectorySearchDlg dialog

using namespace ADODB;

IMPLEMENT_DYNAMIC(CPharmacyDirectorySearchDlg, CNxDialog)

CPharmacyDirectorySearchDlg::CPharmacyDirectorySearchDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(CPharmacyDirectorySearchDlg::IDD, pParent)
{
	EnableDragHandle(false);	// (j.armen 2012-05-30 11:46) - PLID 49854 - Drag handle doesn't look good here.  Disable it.
	m_bMultiMode = FALSE;
	m_nSelectedID = -1;
	m_nTotalAdded = 0;
}

CPharmacyDirectorySearchDlg::~CPharmacyDirectorySearchDlg()
{
}

void CPharmacyDirectorySearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CNxDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ADD, m_nxibAdd);
	DDX_Control(pDX, IDCANCEL, m_nxibClose);
	DDX_Control(pDX, IDC_NXCOLORCTRL1, m_bkgColor);
	DDX_Control(pDX, IDC_LBL_NCPDPID, m_nxsNCPDPID);
	DDX_Control(pDX, IDC_LBL_STORENAME, m_nxsStoreName);
	DDX_Control(pDX, IDC_LBL_CITY, m_nxsCity);
	DDX_Control(pDX, IDC_LBL_STATE, m_nxsState);
	DDX_Control(pDX, IDC_LBL_ZIP, m_nxsZip);
	DDX_Control(pDX, IDC_LBL_PHONE, m_nxsPhone);
	DDX_Control(pDX, IDC_EDIT_NCPDPID_DIR, m_nxeditNCPDPID);
	DDX_Control(pDX, IDC_EDIT_STORENAME_DIR, m_nxeditStoreName);
	DDX_Control(pDX, IDC_EDIT_CITY_DIR, m_nxeditCity);
	DDX_Control(pDX, IDC_EDIT_STATE_DIR, m_nxeditState);
	DDX_Control(pDX, IDC_EDIT_ZIP_DIR, m_nxeditZip);
	DDX_Control(pDX, IDC_EDIT_PHONE_DIR, m_nxeditPhone);
	DDX_Control(pDX, IDC_BTN_REFRESHDIR, m_nxibRefresh);
	DDX_Control(pDX, IDC_LBL_LAST_UPDATE, m_nxsLastUpdated);
	DDX_Control(pDX, IDC_GROUPBOX_PHARMSEARCH, m_nxbGroupBox);
	DDX_Control(pDX, IDC_EDIT_ADDRESS_DIR, m_nxeditAddress);
	DDX_Control(pDX, IDC_EDIT_CROSS_STREET_DIR, m_nxeditCrossStreet);
}

BEGIN_MESSAGE_MAP(CPharmacyDirectorySearchDlg, CNxDialog)
	ON_BN_CLICKED(IDC_ADD, &CPharmacyDirectorySearchDlg::OnBnClickedAdd)
	ON_BN_CLICKED(IDCANCEL, &CPharmacyDirectorySearchDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BTN_REFRESHDIR, &CPharmacyDirectorySearchDlg::OnBnClickedBtnRefresh)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CPharmacyDirectorySearchDlg message handlers
BOOL CPharmacyDirectorySearchDlg::OnInitDialog()
{
	try {
		CNxDialog::OnInitDialog();

		m_bkgColor.SetColor(GetNxColor(GNC_ADMIN, 0));

		if (m_bMultiMode) {
			m_nxibAdd.SetWindowText("&Add");
			m_nxibClose.SetWindowText("&Close");
			m_nxibClose.AutoSet(NXB_CLOSE);
			m_nxibAdd.AutoSet(NXB_NEW);
		} else {
			m_nxibClose.AutoSet(NXB_CANCEL);
			m_nxibAdd.AutoSet(NXB_NEW); // this was a toss up between OK and NEW, but I think this makes more sense
		}
		m_nxibRefresh.AutoSet(NXB_REFRESH);

		//TES 2/8/2013 - PLID 55085 - Dropdown list of the different specialties available to filter on
		m_pSpecialties = BindNxDataList2Ctrl(IDC_SPECIALTY_DIR);

		LoadSettings();
		// (r.gonet 2016-02-25 16:14) - PLID 67961 - Separate the mail order pharmacies from the real search results.
		CString strSearchResultsWhere = GetWhereClause(false);
		CString strMailOrderWhere = GetWhereClause(true);
		// (a.walling 2009-07-13 11:19) - PLID 34861 - The change to only show v4.2 pharmacies broke the default state functionality below
		// on production systems, displaying all pharmacies in the country would take way, way too long.
		//TES 1/9/2013 - PLID 51716 - Renamed PharmacyDirectoryT to NexERxPharmacyT
		//TES 1/23/2013 - PLID 51716 - Also need to update to version 10.6
		// (j.fouts 2013-07-05 09:33) - PLID 57454 - Removed the version check, the default now is empty string
		if (strSearchResultsWhere.IsEmpty() || strMailOrderWhere.IsEmpty()) {
			_RecordsetPtr prsInitial = CreateRecordsetStd("SELECT TOP 1 State FROM LocationsT WHERE Managed = 1 ORDER BY ID ASC");
			if (!prsInitial->eof) {
				m_nxeditState.SetWindowText(AdoFldString(prsInitial, "State", ""));
				strSearchResultsWhere = GetWhereClause(false);
				// (r.gonet 2016-02-25 16:14) - PLID 67961 - Separate the mail order pharmacies from the real search results.
				strMailOrderWhere = GetWhereClause(true);
			}
		}
		
		//extern CPracticeApp theApp;
		//m_nxsLastUpdated.SetFont(theApp.GetPracticeFont(CPracticeApp::pftGeneralBold));

		{
			CString strLastUpdateText;

			_RecordsetPtr prsUpdate = CreateRecordsetStd("SELECT MAX(Date) AS LastUpdate FROM PharmacyDirectoryUpdateHistoryT");
			if (!prsUpdate->eof) {
				_variant_t varDate = prsUpdate->Fields->Item["LastUpdate"]->Value;

				if (varDate.vt == VT_DATE) {
					COleDateTime dtDate = VarDateTime(varDate);

					strLastUpdateText.Format("Blue rows already exist in Nextech. Directory last updated on %s", FormatDateTimeForInterface(dtDate, NULL, dtoDate));
				}
			}

			if (!strLastUpdateText.IsEmpty()) {
				m_nxsLastUpdated.SetWindowText(strLastUpdateText);
			} else {
				m_nxsLastUpdated.SetWindowText("Directory has not been updated");
				MessageBox("The pharmacy directory has not been initially updated. This process should occur automatically on the server on a daily basis.\r\n\r\n"
					"If you continue to see this message, please contact NexTech Technical Support.", NULL, MB_ICONEXCLAMATION);
			}
		}

		m_pList = BindNxDataList2Ctrl(IDC_LIST, false);
		// (r.gonet 2016-02-25 16:14) - PLID 67961 - Mail Order pharmacies are now separate from the actual search results in order to not clutter the results list with
		// non-matches.
		m_pMailOrderPharmaciesList = BindNxDataList2Ctrl(IDC_MAIL_ORDER_PHARMACIES_LIST, false);
		
		if (m_bMultiMode) {
			m_pList->AllowMultiSelect = VARIANT_TRUE;
			// (r.gonet 2016-02-25 16:14) - PLID 67961 - Allow multi selection in the mail order pharmacies list as well.
			m_pMailOrderPharmaciesList->AllowMultiSelect = VARIANT_TRUE;
		}

		if (!strSearchResultsWhere.IsEmpty()) {
			m_pList->PutWhereClause((LPCTSTR)strSearchResultsWhere);
			m_pList->Requery();
		}
		// (r.gonet 2016-02-25 16:14) - PLID 67961 - Separate the mail order pharmacies from the real search results.
		if(!strMailOrderWhere.IsEmpty()) {
			m_pMailOrderPharmaciesList->PutWhereClause((LPCTSTR)strMailOrderWhere);
			m_pMailOrderPharmaciesList->Requery();
		}

	} NxCatchAll(__FUNCTION__);
	return TRUE;
}

CString CPharmacyDirectorySearchDlg::GetWhereClause(bool bMailOrderOnly)
{
	//TES 1/9/2013 - PLID 51716 - Renamed PharmacyDirectoryT to NexERxPharmacyT throughout this function
	//TES 1/23/2013 - PLID 51716 - Also need to update to version 10.6
	// (j.fouts 2013-07-05 09:33) - PLID 57454 - Removed the version filter, we can prescribe to pharmacies
	// on previous versions. b.savon says we do not need to filter out future versions.
	// (a.walling 2014-01-22 16:56) - PLID 59839 - Pharmacy search does not trim spaces; a stray space in the phone number field can cause nothing to return, ever.
	// therefore all values are trimmed now (except specialty)
	CString strWhere = "";

	CString strValue;

	m_nxeditNCPDPID.GetWindowText(strValue);
	strValue = Trim(strValue);
	if (!strValue.IsEmpty()) {
		strWhere += FormatString(" AND (NexERxPharmacyT.NCPDPID LIKE '%s%%')", _Q(strValue));
	}

	m_nxeditStoreName.GetWindowText(strValue);
	strValue = Trim(strValue);
	if (!strValue.IsEmpty()) {
		strWhere += FormatString(" AND (NexERxPharmacyT.StoreName LIKE '%s%%')", _Q(strValue));
	}

	if (!bMailOrderOnly) {
		m_nxeditCity.GetWindowText(strValue);
		strValue = Trim(strValue);
		if (!strValue.IsEmpty()) {
			//TES 4/8/2013 - PLID 56128 - Mail Order pharmacies should never be filtered out based on geographical location
			strWhere += FormatString(" AND (NexERxPharmacyT.City LIKE '%s%%')", _Q(strValue));
		}
	} else {
		// (r.gonet 2016-02-25 16:14) - PLID 67961 - Mail order pharmacies do not filter on the city.
	}

	if (!bMailOrderOnly) {
		m_nxeditState.GetWindowText(strValue);
		strValue = Trim(strValue);
		if (!strValue.IsEmpty()) {
			//TES 4/8/2013 - PLID 56128 - Mail Order pharmacies should never be filtered out based on geographical location
			strWhere += FormatString(" AND (NexERxPharmacyT.State LIKE '%s%%')", _Q(strValue));
		}
	} else {
		// (r.gonet 2016-02-25 16:14) - PLID 67961 - Mail order pharmacies do not filter on the state.
	}

	if (!bMailOrderOnly) {
		m_nxeditZip.GetWindowText(strValue);
		strValue = Trim(strValue);
		if (!strValue.IsEmpty()) {
			//TES 4/8/2013 - PLID 56128 - Mail Order pharmacies should never be filtered out based on geographical location
			strWhere += FormatString(" AND (NexERxPharmacyT.Zip LIKE '%s%%')", _Q(strValue));
		}
	} else {
		// (r.gonet 2016-02-25 16:14) - PLID 67961 - Mail order pharmacies do not filter on the zip.
	}

	if (!bMailOrderOnly) {
		//TES 1/22/2013 - PLID 54723 - Added Street Address
		m_nxeditAddress.GetWindowText(strValue);
		strValue = Trim(strValue);
		if (!strValue.IsEmpty()) {
			//TES 1/22/2013 - PLID 54723 - Note that unlike the other fields, we filter on anywhere in the value, not just in the beginning.
			// We're assuming that they're most likely searching for a street name, without knowing the actual address number.
			//TES 4/8/2013 - PLID 56128 - Mail Order pharmacies should never be filtered out based on geographical location
			strWhere += FormatString(" AND (NexERxPharmacyT.AddressLine1 LIKE '%%%s%%')", _Q(strValue));
		}
	} else {
		// (r.gonet 2016-02-25 16:14) - PLID 67961 - Mail order pharmacies do not filter on the address.
	}

	// (j.jones 2016-02-23 14:16) - PLID 67986 - added support for cross streets
	if (!bMailOrderOnly) {
		m_nxeditCrossStreet.GetWindowText(strValue);
		strValue = Trim(strValue);

		//cross streets are tricky - if you type in two street names, or part of two names,
		//there's no telling what order they are in within the official list, so we really need to
		//search if any words they type in, in any order, exist in any order in the CrossStreet field
		
		//turn all spaces into a single space
		while(strValue.Replace("  ", " ") > 0);

		if (!strValue.IsEmpty()) {

			CStringArray aryKeywords;
			ParseDelimitedStringToStringArray(strValue, " ", aryKeywords);

			if (aryKeywords.GetCount() > 0) {

				CString strCurWhere = "";

				for (int i = 0; i < aryKeywords.GetCount(); i++)
				{
					CString strKeyword = aryKeywords.GetAt(i);
					strKeyword = Trim(strKeyword);

					if (!strKeyword.IsEmpty()) {
						if (!strCurWhere.IsEmpty()) {
							strCurWhere += " AND ";
						}
						strCurWhere += FormatString("NexERxPharmacyT.CrossStreet LIKE '%%%s%%'", _Q(strKeyword));
					}
				}
								
				strWhere += " AND (" + strCurWhere + ")";
			}
		}
	} else {
		// (r.gonet 2016-02-25 16:14) - PLID 67961 - Mail order pharmacies do not filter on the cross street.
	}

	//TES 2/8/2013 - PLID 55085 - Added Specialty.  We have filtered the dropdown list to only include specialties
	// that are in the list, so we will search on the exact text selected, in any of the 4 specialty type fields.
	NXDATALIST2Lib::IRowSettingsPtr pSpecialty = m_pSpecialties->CurSel;
	if(pSpecialty) {
		CString strSpecialty = VarString(pSpecialty->GetValue(scSpecialty),"");
		if(!strSpecialty.IsEmpty()) {
			strWhere += FormatString(" AND ("
				"NexERxPharmacyT.SpecialtyType1 = '%s' OR "
				"NexERxPharmacyT.SpecialtyType2 = '%s' OR "
				"NexERxPharmacyT.SpecialtyType3 = '%s' OR "
				"NexERxPharmacyT.SpecialtyType4 = '%s'"
				") ",
				_Q(strSpecialty), _Q(strSpecialty), _Q(strSpecialty), _Q(strSpecialty));
		}
	}


	m_nxeditPhone.GetWindowText(strValue);
	strValue = Trim(strValue);
	if (!strValue.IsEmpty()) {
		strWhere += FormatString(" AND ("
			"NexERxPharmacyT.PhonePrimary LIKE '%s%%' OR "
			"NexERxPharmacyT.Fax LIKE '%s%%' OR "
			"NexERxPharmacyT.PhoneAlt1 LIKE '%s%%' OR "
			"NexERxPharmacyT.PhoneAlt2 LIKE '%s%%' OR "
			"NexERxPharmacyT.PhoneAlt3 LIKE '%s%%' OR "
			"NexERxPharmacyT.PhoneAlt4 LIKE '%s%%' OR "
			"NexERxPharmacyT.PhoneAlt5 LIKE '%s%%' "
			")", 
			_Q(strValue),
			_Q(strValue),
			_Q(strValue),
			_Q(strValue),
			_Q(strValue),
			_Q(strValue),
			_Q(strValue)
			);
	}

	if (bMailOrderOnly) {
		//TES 1/22/2013 - PLID 54723 - Note that unlike the other fields, we filter on anywhere in the value, not just in the beginning.
		// We're assuming that they're most likely searching for a street name, without knowing the actual address number.
		//TES 4/8/2013 - PLID 56128 - Mail Order pharmacies should never be filtered out based on geographical location
		// (r.gonet 2016-02-25 16:14) - PLID 67961 - Pull in only mail order pharmacies.
		strWhere += FormatString(" AND (SpecialtyType1 = 'MailOrder' OR SpecialtyType2 = 'MailOrder' OR SpecialtyType3 = 'MailOrder' OR SpecialtyType4 = 'MailOrder')");
	} else {
		// (r.gonet 2016-02-25 16:14) - PLID 67961 - Allow any type of pharmacy that matches the search results.
	}

	strWhere.TrimLeft(" AND");

	// (j.fouts 2013-07-05 10:52) - PLID 57454 - This can now be empty
	if(strWhere.IsEmpty())
	{
		return "";
	}
	else
	{
		return CString("(") + strWhere + CString(")");
	}
}

void CPharmacyDirectorySearchDlg::LoadSettings()
{
	// (a.walling 2014-01-22 16:56) - PLID 59839 - Trim values while loading
	g_propManager.CachePropertiesInBulk("PharmacyDirectorySearch", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'PharmacyDirectorySearch_NCPDPID' OR "
			"Name = 'PharmacyDirectorySearch_StoreName' OR "
			"Name = 'PharmacyDirectorySearch_City' OR "
			"Name = 'PharmacyDirectorySearch_State' OR "
			"Name = 'PharmacyDirectorySearch_Zip' OR "
			"Name = 'PharmacyDirectorySearch_Phone' OR "
			"Name = 'PharmacyDirectorySearch_Address' OR "//TES 1/22/2013 - PLID 54723
			"Name = 'PharmacyDirectorySearch_Specialty' OR " //TES 2/8/2013 - PLID 55085
			"Name = 'PharmacyDirectorySearch_CrossStreet' " // (j.jones 2016-02-23 14:16) - PLID 67986 - added support for cross streets
			")", _Q(GetCurrentUserName())
		);

	m_nxeditNCPDPID.SetWindowText(Trim(GetRemotePropertyText("PharmacyDirectorySearch_NCPDPID", "", 0, "<None>", true)));
	m_nxeditStoreName.SetWindowText(Trim(GetRemotePropertyText("PharmacyDirectorySearch_StoreName", "", 0, "<None>", true)));
	//TES 1/22/2013 - PLID 54723 - Added Street Address
	m_nxeditAddress.SetWindowText(Trim(GetRemotePropertyText("PharmacyDirectorySearch_Address", "", 0, "<None>", true)));
	// (j.jones 2016-02-23 14:16) - PLID 67986 - added support for cross streets
	m_nxeditCrossStreet.SetWindowText(Trim(GetRemotePropertyText("PharmacyDirectorySearch_CrossStreet", "", 0, "<None>", true)));
	m_nxeditCity.SetWindowText(Trim(GetRemotePropertyText("PharmacyDirectorySearch_City", "", 0, "<None>", true)));
	m_nxeditState.SetWindowText(Trim(GetRemotePropertyText("PharmacyDirectorySearch_State", "", 0, "<None>", true)));
	m_nxeditZip.SetWindowText(Trim(GetRemotePropertyText("PharmacyDirectorySearch_Zip", "", 0, "<None>", true)));
	m_nxeditPhone.SetWindowText(Trim(GetRemotePropertyText("PharmacyDirectorySearch_Phone", "", 0, "<None>", true)));
	//TES 2/8/2013 - PLID 55085 - Added Specialty
	m_pSpecialties->SetSelByColumn(scSpecialty, _bstr_t(GetRemotePropertyText("PharmacyDirectorySearch_Specialty", "", 0, "<None>", true)));
}

void CPharmacyDirectorySearchDlg::SaveSettings()
{
	// (a.walling 2014-01-22 16:56) - PLID 59839 - Trim values before saving
	CString strValue;

	m_nxeditNCPDPID.GetWindowText(strValue);
	SetRemotePropertyText("PharmacyDirectorySearch_NCPDPID", Trim(strValue), 0, "<None>");

	m_nxeditStoreName.GetWindowText(strValue);
	SetRemotePropertyText("PharmacyDirectorySearch_StoreName", Trim(strValue), 0, "<None>");

	//TES 1/22/2013 - PLID 54723 - Added Street Address
	m_nxeditAddress.GetWindowText(strValue);
	SetRemotePropertyText("PharmacyDirectorySearch_Address", Trim(strValue), 0, "<None>");

	// (j.jones 2016-02-23 14:16) - PLID 67986 - added support for cross streets
	m_nxeditCrossStreet.GetWindowText(strValue);
	SetRemotePropertyText("PharmacyDirectorySearch_CrossStreet", Trim(strValue), 0, "<None>");

	m_nxeditCity.GetWindowText(strValue);
	SetRemotePropertyText("PharmacyDirectorySearch_City", Trim(strValue), 0, "<None>");

	m_nxeditState.GetWindowText(strValue);
	SetRemotePropertyText("PharmacyDirectorySearch_State", Trim(strValue), 0, "<None>");

	m_nxeditZip.GetWindowText(strValue);
	SetRemotePropertyText("PharmacyDirectorySearch_Zip", Trim(strValue), 0, "<None>");

	m_nxeditPhone.GetWindowText(strValue);
	SetRemotePropertyText("PharmacyDirectorySearch_Phone", Trim(strValue), 0, "<None>");

	//TES 2/8/2013 - PLID 55085 - Added Specialty
	CString strSpecialty;
	NXDATALIST2Lib::IRowSettingsPtr pSpecialty = m_pSpecialties->CurSel;
	if(pSpecialty) {
		strSpecialty = VarString(pSpecialty->GetValue(scSpecialty), "");
	}
	SetRemotePropertyText("PharmacyDirectorySearch_Specialty", strSpecialty, 0, "<None>");
}

struct PharmacyComparator {
	bool operator() (const NexTech_Accessor::_PharmacyImportInputPtr& lhs, const NexTech_Accessor::_PharmacyImportInputPtr& rhs) const
	{
		return strcmp(lhs->NCPDPID, rhs->NCPDPID) < 0;
	}
};

void CPharmacyDirectorySearchDlg::OnBnClickedAdd()
{
	try {
		m_nSelectedID = -1;
			
		CWaitCursor cwait;
		bool bWillOverwrite = false;

		//	Go through our list and pick off the ones the user has selected to import.
		//TES 11/15/2012 - PLID 53807 - Prepare an array of objects to pass to the API
		// (r.gonet 2016-02-25 16:42) - PLID 67961 - Changed to a set.
		std::set<NexTech_Accessor::_PharmacyImportInputPtr, PharmacyComparator> setPharmaciesNeedingImport;
		long nSelected = 0;

		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->GetFirstSelRow();
		while (pRow) {
			nSelected++;

			NexTech_Accessor::_PharmacyImportInputPtr pharmacy(__uuidof(NexTech_Accessor::PharmacyImportInput));
				
			//TES 11/15/2012 - PLID 53807 - Is this pharmacy already in the list?
			_variant_t varLocationID = pRow->GetValue(lcLocationID);
			if (varLocationID.vt == VT_I4) {
				if (setPharmaciesNeedingImport.size() == 0) {
					m_nSelectedID = VarLong(varLocationID);
				}
			} else {
				pharmacy->PharmacyName = _bstr_t(pRow->GetValue(lcStoreName));
				pharmacy->NCPDPID = _bstr_t(pRow->GetValue(lcNCPDPID));
				if (!setPharmaciesNeedingImport.insert(pharmacy).second) {
					// (r.gonet 2016-02-25 16:46) - PLID 67961 - It already existed in the set.
					nSelected--;
				}
			}
			
			pRow = pRow->GetNextSelRow();
		}

		// (r.gonet 2016-02-25 16:14) - PLID 67961 - Also iterate over the mail order pharmacies in case some of those are selected too.
		pRow = m_pMailOrderPharmaciesList->GetFirstSelRow();
		while (pRow) {
			nSelected++;

			NexTech_Accessor::_PharmacyImportInputPtr pharmacy(__uuidof(NexTech_Accessor::PharmacyImportInput));

			//TES 11/15/2012 - PLID 53807 - Is this pharmacy already in the list?
			_variant_t varLocationID = pRow->GetValue(lcLocationID);
			if (varLocationID.vt == VT_I4) {
				if (setPharmaciesNeedingImport.size() == 0) {
					m_nSelectedID = VarLong(varLocationID);
				}
			} else {
				pharmacy->PharmacyName = _bstr_t(pRow->GetValue(lcStoreName));
				pharmacy->NCPDPID = _bstr_t(pRow->GetValue(lcNCPDPID));
				if (!setPharmaciesNeedingImport.insert(pharmacy).second) {
					// (r.gonet 2016-02-25 16:46) - PLID 67961 - It already existed in the set.
					nSelected--;
				}
			}

			pRow = pRow->GetNextSelRow();
		}

		// (j.jones 2016-02-29 10:48) - PLID 67987 - if nothing was selected, say so
		if (nSelected <= 0) {
			AfxMessageBox("No pharmacies have been selected. Please select one or more new pharmacies to import.");
			return;
		}

		// (j.jones 2016-02-24 09:23) - PLID 67987 - if all the selected pharmacies have already been imported,
		// don't tell them they have to select new ones, just continue on - they clearly want to use that pharmacy
		if (setPharmaciesNeedingImport.size() > 0) {

			//some pharmacies need imported

			// sanity check
			if (setPharmaciesNeedingImport.size() > 12) {
				if (IDYES != MessageBox(FormatString("Are you sure you want to import all %li new pharmacies into Practice?", setPharmaciesNeedingImport.size()), NULL, MB_ICONQUESTION | MB_YESNO)) {
					return;
				}
			}
			
			// (r.gonet 2016-02-25 16:44) - PLID 67961 - Switch to an array now that we've ensure there are no duplicates.
			CArray<NexTech_Accessor::_PharmacyImportInputPtr, NexTech_Accessor::_PharmacyImportInputPtr> aryPharmaciesNeedingImport;
			for each(NexTech_Accessor::_PharmacyImportInputPtr pPharmacy in setPharmaciesNeedingImport)
			{
				aryPharmaciesNeedingImport.Add(pPharmacy);
			}

			//TES 11/15/2012 - PLID 53807 - Create our SAFEARRAY to be passed to the PharmacyImport function in the API
			Nx::SafeArray<IUnknown *> saryPharmacies = Nx::SafeArray<IUnknown *>::From(aryPharmaciesNeedingImport);

			//TES 11/15/2012 - PLID 53807 - Call the API to import the pharmacies and then convert the handed back SAFEARRAY to a CArray so we can do something useful with it.
			NexTech_Accessor::_PharmacyImportResultsArrayPtr importResults = GetAPI()->PharmacyImport(GetAPISubkey(), GetAPILoginToken(), saryPharmacies);

			//TES 11/15/2012 - PLID 53807 - If for some reason we get nothing back (although, this should never happen), tell the user and bail.
			if (importResults->PharmacyImportResults == NULL) {
				MsgBox("There were no pharmacies to import.");
				return;
			}

			Nx::SafeArray<IUnknown *> saryPharmacyResults(importResults->PharmacyImportResults);

			//	Prepare our results message so the user knows which imported succesfully (or not)
			CString strSuccess = "The following pharmacies have imported successfully:\r\n";
			CString strFailure = "\r\nThe following pharmacies are already imported:\r\n";
			//TES 2/11/2013 - PLID 55111 - It is now possible for the ImportPharmacy function to return values indicating that the given NCPDPID was not in
			// the directory.  That shouldn't ever happen hear, since we're getting the value from the directory, but I added handling for completeness.
			CString strNotFound = "\r\nThe following pharmacies could not be found in the directory:\r\n";
			long nAdded = 0;
			//TES 11/15/2012 - PLID 53807 - Go through the objects the API returned to us.
			foreach(NexTech_Accessor::_PharmacyImportOutputPtr pPharmacyResult, saryPharmacyResults)
			{
				if (pPharmacyResult->Success == VARIANT_TRUE) {
					m_nSelectedID = pPharmacyResult->locationID;
					strSuccess += CString((LPCTSTR)pPharmacyResult->PharmacyName) + "\r\n";
					nAdded++;
				}
				else {
					if (pPharmacyResult->locationID == -1) {
						strNotFound += CString((LPCTSTR)pPharmacyResult->PharmacyName) + "\r\n";
					}
					else {
						strFailure += CString((LPCTSTR)pPharmacyResult->PharmacyName) + "\r\n";
					}
				}
			}
			m_nTotalAdded += nAdded;

			CString strResults;
			if (strSuccess != "The following pharmacies have imported successfully:\r\n") {
				strResults += strSuccess;
			}

			if (strFailure != "\r\nThe following pharmacies are already imported:\r\n") {
				strResults += strFailure;
			}

			if (strNotFound != "\r\nThe following pharmacies could not be found in the directory:\r\n") {
				strResults += strNotFound;
			}

			if (nAdded == 1) {
				CClient::RefreshTable(NetUtils::LocationsT, m_nSelectedID);
			}
			else if (nAdded > 1) {
				CClient::RefreshTable(NetUtils::LocationsT, -1);
			}

			//if in multi-select mode, we're not closing the dialog,
			//so refresh the lists to show that we imported pharmacies
			if (m_bMultiMode && nSelected > 0) {
				if (!strResults.IsEmpty()) {
					MsgBox("%s", strResults);
					m_pList->Requery();
					// (r.gonet 2016-02-25 16:14) - PLID 67961 - Requery the mail order pharamacies too.
					m_pMailOrderPharmaciesList->Requery();
				}
			}
		}

		//if in single-select mode, OnOK selects the desired pharmacy

		// if we are not in 'multi' mode, then we want to select this new pharmacy (or the existing one). m_nSelectedID should be set.
		if (!m_bMultiMode && nSelected > 0) {
			OnOK();
		}

	}NxCatchAll(__FUNCTION__);
}

void CPharmacyDirectorySearchDlg::OnBnClickedCancel()
{
	try {
		if (!m_bMultiMode) {
			OnCancel();
		} else {
			if (m_nTotalAdded > 0) {
				OnOK();
			} else {
				OnCancel();
			}
		}
	} NxCatchAll(__FUNCTION__);
}

void CPharmacyDirectorySearchDlg::OnBnClickedBtnRefresh()
{
	try {
		CString strSearchResultsWhere = GetWhereClause(false);
		m_pList->PutWhereClause((LPCTSTR)strSearchResultsWhere);
		m_pList->Requery();

		// (r.gonet 2016-02-25 16:14) - PLID 67961 - Reload the mail order pharmacies too.
		CString strMailOrderWhere = GetWhereClause(true);
		m_pMailOrderPharmaciesList->PutWhereClause((LPCTSTR)strMailOrderWhere);
		m_pMailOrderPharmaciesList->Requery();

	} NxCatchAll(__FUNCTION__);
}

void CPharmacyDirectorySearchDlg::OnDestroy()
{
	try {
		SaveSettings();
	} NxCatchAll(__FUNCTION__);

	CNxDialog::OnDestroy();
}

BEGIN_EVENTSINK_MAP(CPharmacyDirectorySearchDlg, CNxDialog)
	ON_EVENT(CPharmacyDirectorySearchDlg, IDC_LIST, 3, CPharmacyDirectorySearchDlg::DblClickCellList, VTS_DISPATCH VTS_I2)
	ON_EVENT(CPharmacyDirectorySearchDlg, IDC_LIST, 28, CPharmacyDirectorySearchDlg::CurSelWasSetList, VTS_NONE)
	ON_EVENT(CPharmacyDirectorySearchDlg, IDC_MAIL_ORDER_PHARMACIES_LIST, 28, CPharmacyDirectorySearchDlg::CurSelWasSetMailOrderPharmaciesList, VTS_NONE)
	ON_EVENT(CPharmacyDirectorySearchDlg, IDC_MAIL_ORDER_PHARMACIES_LIST, 3, CPharmacyDirectorySearchDlg::DblClickCellMailOrderPharmaciesList, VTS_DISPATCH VTS_I2)
END_EVENTSINK_MAP()

void CPharmacyDirectorySearchDlg::DblClickCellList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		OnBnClickedAdd();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-02-25 16:14) - PLID 67961 - Handle double clicking on the mail order pharmacy rows.
void CPharmacyDirectorySearchDlg::DblClickCellMailOrderPharmaciesList(LPDISPATCH lpRow, short nColIndex)
{
	try {
		OnBnClickedAdd();
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-02-25 16:14) - PLID 67961 - Handle when the current selection is set in the search results list.
void CPharmacyDirectorySearchDlg::CurSelWasSetList()
{
	try {
		// By checking if CTRL is down, the selection becomes more natural. It is as though we are selecting from one list instead of two.
		BOOL bIsCtrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x80000000);

		if (!m_bMultiMode || !bIsCtrlDown) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pList->CurSel;
			if (pRow == nullptr) {
				// (r.gonet 2016-02-25 16:14) - PLID 67961 - Nothing to do.
				return;
			}

			// (r.gonet 2016-02-25 16:14) - PLID 67961 - Limit it to either a selection from the search results list or a selection from the mail order list.
			// Otherwise, the Add button won't know which to use.
			NXDATALIST2Lib::IRowSettingsPtr pOtherListRow = m_pMailOrderPharmaciesList->CurSel;
			if (pOtherListRow != nullptr) {
				// (r.gonet 2016-02-25 16:14) - PLID 67961 - Unselect the selection from the mail order pharmacies list.
				m_pMailOrderPharmaciesList->CurSel = nullptr;
			} else {
				// (r.gonet 2016-02-25 16:14) - PLID 67961 - Nothing was selected in the mail order pharmacies list. Nothing needs to be unselected.
			}
		} else {
			// (r.gonet 2016-02-25 16:14) - PLID 67961 - Allow selections to be made in both lists.
		}
	} NxCatchAll(__FUNCTION__);
}

// (r.gonet 2016-02-25 16:14) - PLID 67961 - Handle when the current selection is set in the mail order pharmacies list.
void CPharmacyDirectorySearchDlg::CurSelWasSetMailOrderPharmaciesList()
{
	try {
		// By checking if CTRL is down, the selection becomes more natural. It is as though we are selecting from one list instead of two.
		BOOL bIsCtrlDown = (GetAsyncKeyState(VK_CONTROL) & 0x80000000);

		if (!m_bMultiMode || !bIsCtrlDown) {
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pMailOrderPharmaciesList->CurSel;
			if (pRow == nullptr) {
				// (r.gonet 2016-02-25 16:14) - PLID 67961 - Nothing to do.
				return;
			}

			// (r.gonet 2016-02-25 16:14) - PLID 67961 - Limit it to either a selection from the search results list or a selection from the mail order list.
			// Otherwise, the Add button won't know which to use.
			NXDATALIST2Lib::IRowSettingsPtr pOtherListRow = m_pList->CurSel;
			if (pOtherListRow != nullptr) {
				// (r.gonet 2016-02-25 16:14) - PLID 67961 - Unselect the selection from the search results list.
				m_pList->CurSel = nullptr;
			} else {
				// (r.gonet 2016-02-25 16:14) - PLID 67961 - Nothing was selected in the search results list. Nothing needs to be unselected.
			}
		} else {
			// (r.gonet 2016-02-25 16:14) - PLID 67961 - Allow selections to be made in both lists.
		}
	} NxCatchAll(__FUNCTION__);
}