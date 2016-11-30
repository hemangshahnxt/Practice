// CHL7ToBeBilledDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Practice.h"
#include "HL7ToBeBilledDlg.h"
#include "HL7ParseUtils.h"
#include "afxdialogex.h"
#include "HL7Utils.h"
#include <NxUILib\NxMenuCommandMap.h>
#include <NxHL7Lib/HL7MessageFactory.h>
#include <NxHL7Lib/HL7CommonUtils.h>
#include <NxHL7Lib/Hl7Logging.h>
#include "BillingModuleDlg.h"

//(j.camacho 2016-01-27) PLID 68000 - Create HL7 Bill Dialog
// CHL7ToBeBilledDlg dialog

#define ALL_PROVIDERS "<All Providers>"
#define ALL_LOCATIONS "<All Locations>"

// (j.gruber 2016-02-01 13:40) - PLID 68000 - Create a dialog that lists any uncommitted HL7 bill messages in a datalist
enum BillListColumns {

	blcID = 0,
	blcGroupID,
	blcGroupName,
	blcServiceDate,
	blcPersonID,
	blcPatientName,
	blcDescription,
	blcCharges,
	blcDxCodes,
	blcProvider,        
	blcLocation,		
	blcBillVisit,
};

// (j.gruber 2016-02-01 13:41) - PLID 68003 - Create a provider filter on the HL7 visit and make it work
enum ProviderListColumns
{
	plcName = 0,
	plcOrder,
};

// (j.gruber 2016-02-01 13:41) - PLID 68004 - Add a location filter to the HL7 visits dialog
enum LocationListColumns
{
	llcName = 0,
	llcOrder
};

IMPLEMENT_DYNAMIC(CHL7ToBeBilledDlg, CNxDialog)

CHL7ToBeBilledDlg::CHL7ToBeBilledDlg(CWnd* pParent /*=NULL*/)
	: CNxDialog(IDD_HL7_TO_BE_BILLED_DLG, pParent)
{
	SetMinSize(710, 515);
}

CHL7ToBeBilledDlg::~CHL7ToBeBilledDlg()
{
}

void CHL7ToBeBilledDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDOK, m_btnClose);
	DDX_Control(pDX, IDC_HL7_BILL_RELOAD, m_btnRefresh);
	CNxDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CHL7ToBeBilledDlg, CNxDialog)
	ON_BN_CLICKED(IDOK, &CHL7ToBeBilledDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CHL7ToBeBilledDlg::OnCancel)
	ON_BN_CLICKED(IDC_HL7_BILL_RELOAD, &CHL7ToBeBilledDlg::OnBnClickedHl7BillReload)
	ON_BN_CLICKED(IDC_ENABLE_FILTERS, &CHL7ToBeBilledDlg::OnBnClickedEnableFilters)
	ON_WM_SHOWWINDOW()
	ON_WM_NCHITTEST()
END_MESSAGE_MAP()


// CHL7ToBeBilledDlg message handlers

// (j.gruber 2016-02-01 13:42) - PLID 68000 - Create a dialog that lists any uncommitted HL7 bill messages in a datalist
void CHL7ToBeBilledDlg::OnBnClickedOk()
{
	try {

		CNxDialog::OnOK();
		DestroyWindow();
	}NxCatchAll(__FUNCTION__);
	
}

void CHL7ToBeBilledDlg::OnCancel()
{
	try {

		CNxDialog::OnCancel();
		DestroyWindow();
	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2016-02-01 13:42) - PLID 68000 - Create a dialog that lists any uncommitted HL7 bill messages in a datalist
BOOL CHL7ToBeBilledDlg::OnInitDialog()
{
	
	CNxDialog::OnInitDialog();
	

	try {
		//found important Preference for displaying icon if window exists that EMN to be billed follows
		if (GetRemotePropertyInt("DisplayTaskbarIcons", 0, 0, GetCurrentUserName(), true) == 1) {
			HWND hwnd = GetSafeHwnd();
			long nStyle = GetWindowLong(hwnd, GWL_EXSTYLE);
			nStyle |= WS_EX_APPWINDOW;
			SetWindowLong(hwnd, GWL_EXSTYLE, nStyle);
		}

		m_btnClose.AutoSet(NXB_CLOSE);
		m_btnRefresh.AutoSet(NXB_REFRESH);

		// (j.gruber 2016-02-01 13:42) - PLID 68000 - Create a dialog that lists any uncommitted HL7 bill messages in a datalist
		m_pBillList = BindNxDataList2Ctrl(IDC_HL7_BILLS, false);
		// (j.gruber 2016-02-01 13:42) - PLID 68004 - Add a location filter to the HL7 visits dialog
		m_pLocationList = BindNxDataList2Ctrl(IDC_LOCATION_LIST_FILTER, false);
		// (j.gruber 2016-02-01 13:42) - PLID 68003 - Create a provider filter on the HL7 visit and make it work
		m_pProviderList = BindNxDataList2Ctrl(IDC_PROVIDER_LIST_FILTER, false);

		// (j.gruber 2016-02-01 13:42) - PLID 68004 - bulk cache
		// (j.gruber 2016-02-01 13:42) - PLID 68003 - bulk cache
		g_propManager.CachePropertiesInBulk("HL7ToBeBilledDlg", propText,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'HL7ToBeBilledProviderFilter' OR "
			"Name = 'HL7ToBeBilledLocationFilter' "
			")", _Q(GetCurrentUserName()));

		// (j.gruber 2016-02-01 12:17) - PLID 68120 - As a biller, I must be able to enable filters for the Visits To Be Billed dialog.
		g_propManager.CachePropertiesInBulk("HL7ToBeBilledDlg", propNumber,
			"(Username = '<None>' OR Username = '%s') AND ("
			"Name = 'HL7ToBeBilledEnableFilters'"
			")", _Q(GetCurrentUserName()));

	

	}NxCatchAll(__FUNCTION__);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}
//(j.camacho 2016-3-22) Plid 68001 - Should handle grabbing the window
LRESULT CHL7ToBeBilledDlg::OnNcHitTest(CPoint point)
{
	/* Calculate the new position of the size grip */
	CRect rc;
	GetWindowRect(&rc);
	rc.top = rc.bottom - GetSystemMetrics(SM_CYHSCROLL);
	rc.left = rc.right - GetSystemMetrics(SM_CXVSCROLL);

	if (rc.PtInRect(point)) {
		return HTBOTTOMRIGHT;
	}

	return CNxDialog::OnNcHitTest(point);
}
// (j.gruber 2016-02-01 13:43) - PLID 68000 - reload everything when the window shows
void CHL7ToBeBilledDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
	try {
		CNxDialog::OnShowWindow(bShow, nStatus);

		if (bShow)
		{
			long nChecked = GetRemotePropertyInt("HL7ToBeBilledEnableFilters", 0, 0, GetCurrentUserName());
			if (nChecked == 1)
			{
				CheckDlgButton(IDC_ENABLE_FILTERS, 1);
				m_pProviderList->Enabled = true;
				m_pLocationList->Enabled = true;
			}
			else {
				CheckDlgButton(IDC_ENABLE_FILTERS, 0);
				m_pProviderList->Enabled = false;
				m_pLocationList->Enabled = false;
			}

			ReloadMaps(TRUE);
		}
	}NxCatchAll(__FUNCTION__)
}


// (j.gruber 2016-02-01 13:43) - PLID 68000 - main function that reloads our map from data
void CHL7ToBeBilledDlg::ReloadMaps(BOOL bReloadFilterDataLists)
{
	try {

		//first clear the list
		m_mapBills.clear();
		m_mapProviders.clear();
		m_mapLocations.clear();
		boost::unordered_map<std::pair<long, CString>, CString> mapHL7MappedLocations;
		boost::unordered_map<std::pair<long, CString>, CString> mapHL7MappedProviders;
		std::vector<HL7BillPtr> vecHL7Bills;
		std::vector<std::tuple<CString, CString, CString>> vecProvNames;

		//run our query to get all uncommitted HL7 bill messages
		ADODB::_RecordsetPtr rsRecordsets = CreateParamRecordset(
			"SELECT HL7MessageQueueT.ID, GroupID, HL7SettingsT.Name AS GroupName, PatientName, Description, \r\n"
			"	MessageType, EventType, ActionID, InputDate,HL7MessageQueueT.Message, \r\n"
			"	{SQL} AS MessageDateString, \r\n"
			"	CASE WHEN HL7MessageQueueT.PurgeDate IS NULL THEN CONVERT(BIT, 0) ELSE CONVERT(BIT, 1) END AS WasPurged \r\n"
			"FROM HL7MessageQueueT \r\n"
			"INNER JOIN HL7SettingsT ON HL7MessageQueueT.GroupID = HL7SettingsT.ID \r\n"
			"WHERE ActionID Is Null AND (MessageType = 'DFT' AND EventType = 'P03')  \r\n"

			"SELECT HL7CodeLinkT.HL7GroupID, HL7CodeLinkT.ThirdPartyCode, LocationsT.Name \r\n"
			"FROM HL7CodeLinkT \r\n"
			"INNER JOIN LocationsT ON HL7CodeLinkT.PracticeID = LocationsT.ID \r\n"
			"WHERE HL7CodeLinkT.Type = {INT} \r\n"

			"SELECT HL7IDLinkT.GroupID, HL7IDLinkT.ThirdPartyID, PersonT.FullName \r\n"
			"FROM HL7IDLinkT \r\n"
			"INNER JOIN PersonT ON PersonT.ID = HL7IDLinkT.PersonID \r\n"
			"WHERE HL7IDLinkT.RecordType = {INT} \r\n"

			"SELECT PersonT.First, PersonT.Middle, PersonT.Last \r\n"
			"FROM PersonT \r\n"
			"INNER JOIN ProvidersT ON PersonT.ID = ProvidersT.PersonID \r\n"
			, GetMessageDateStringSql(), hclrtLocation, hilrtProvider);
		ADODB::_RecordsetPtr rsBillMessages = rsRecordsets;
		ADODB::_RecordsetPtr rsLocationLinks = rsRecordsets->NextRecordset(NULL);
		ADODB::_RecordsetPtr rsProviderLinks = rsRecordsets->NextRecordset(NULL);
		ADODB::_RecordsetPtr rsProviderNames = rsRecordsets->NextRecordset(NULL);
		while (!rsBillMessages->eof) {

			HL7BillPtr pBill(new HL7Bill);
			pBill->nID = AdoFldLong(rsBillMessages, "ID", -1);
			pBill->nGroupID = AdoFldLong(rsBillMessages, "GroupID", -1);
			pBill->strGroupName = AdoFldString(rsBillMessages, "GroupName", "");
			
			pBill->strPatientName = AdoFldString(rsBillMessages, "PatientName", "");
			pBill->strDescription = AdoFldString(rsBillMessages, "Description", "");
			CString strMessageDateString = AdoFldString(rsBillMessages, "MessageDateString", "");
			CString strMessage = AdoFldString(rsBillMessages, "Message", "");
			pBill->strProvider = GetProviderFromHL7Message(strMessage, pBill->nGroupID);
			pBill->strLocation = GetLocationFromHL7Message(strMessage, pBill->nGroupID, GetHL7SettingBit(pBill->nGroupID, "EnableIntelleChart"), GetHL7SettingBit(pBill->nGroupID, "UseImportedLocation"));
			COleDateTime dtEarliestChargeDate = GetEarliestChargeDateFromHL7Bill(strMessage, pBill->nGroupID);
			pBill->dtServiceDate = dtEarliestChargeDate.GetStatus() == COleDateTime::valid ? dtEarliestChargeDate : GetHL7DateFromStringField(strMessageDateString, pBill->nGroupID);
			long nCountCharges;
			// (j.gruber 2016-02-01 13:47) - PLID 68126 - Charge codes
			pBill->strCharges = GetChargeCodeStringFromHL7Message(strMessage, pBill->nGroupID, nCountCharges);
			pBill->dtInputDate = AdoFldDateTime(rsBillMessages, "InputDate");
			
			// (j.gruber 2016-02-01 13:47) - PLID 68008 - need to try to get personID in order to pop out menu
			pBill->nPersonID = GetPatientFromHL7Message(strMessage, pBill->nGroupID, this, ENotFoundBehavior::nfbSkip);
			pBill->strMessage = strMessage;


			// (j.gruber 2016-02-01 13:47) - PLID 68127 - get the diagnosis codes
			HL7Message Message;
			Message.nMessageID = pBill->nID;
			Message.strMessage = pBill->strMessage;
			Message.nHL7GroupID = pBill->nGroupID;
			Message.strHL7GroupName = pBill->strGroupName;
			Message.strPatientName = pBill->strPatientName;
			Message.dtInputDate = pBill->dtInputDate;
			
			pBill->strDxCodes = GetDiagCodeStringFromHL7Message(Message, pBill->nGroupID, 			
				GetHL7SettingText(pBill->nGroupID, "DefaultCodeSystem"),
				GetHL7SettingText(pBill->nGroupID, "DefaultAltCodeSystem"),
				GetHL7SettingText(pBill->nGroupID, "CodeSystemFlag9"),
				GetHL7SettingText(pBill->nGroupID, "CodeSystemFlag10"));		
			
			//add the HL7 bill to a vector for later use
			vecHL7Bills.push_back(pBill);

			rsBillMessages->MoveNext();
		}

		// store the HL7 group ID and third party IDs that are mapped to Practice locations in a map for later use
		while (!rsLocationLinks->eof)
		{
			long nGroupID = AdoFldLong(rsLocationLinks, "HL7GroupID", -1);
			CString strThirdPartyCode = AdoFldString(rsLocationLinks, "ThirdPartyCode", "");
			CString strLocName = AdoFldString(rsLocationLinks, "Name", "");
			mapHL7MappedLocations.insert(std::make_pair(std::make_pair(nGroupID, strThirdPartyCode), strLocName));
			rsLocationLinks->MoveNext();
		}

		// store the HL7 group ID and third party IDs that are mapped to Practice providers in a map for later use
		while (!rsProviderLinks->eof)
		{
			long nGroupID = AdoFldLong(rsProviderLinks, "GroupID", -1);
			CString strThirdPartyCode = AdoFldString(rsProviderLinks, "ThirdPartyID", "");
			CString strProvName = AdoFldString(rsProviderLinks, "FullName", "");
			mapHL7MappedProviders.insert(std::make_pair(std::make_pair(nGroupID, strThirdPartyCode), strProvName));
			rsProviderLinks->MoveNext();
		}

		// store all provider names for later use, in the event that the provider can not be mapped via a third party ID
		while (!rsProviderNames->eof)
		{
			CString strFirst = AdoFldString(rsProviderNames, "First", "");
			CString strMiddle = AdoFldString(rsProviderNames, "Middle", "");
			CString strLast = AdoFldString(rsProviderNames, "Last", "");
			vecProvNames.push_back(std::make_tuple(strFirst, strMiddle, strLast));
			rsProviderNames->MoveNext();
		}

		// add the contents of the bill vector to the member variable maps, after the vector's bill contents have had their location/providers appropriately
		// changed, if applicable.
		 for(HL7BillPtr pBill : vecHL7Bills)
		 {
			 // attempt to map the bill's location and provider to a Practice location and provider
			 MapHL7ProviderOrLocationField(pBill, mapHL7MappedLocations, vecProvNames, true);
			 MapHL7ProviderOrLocationField(pBill, mapHL7MappedProviders, vecProvNames, false);

			 //add it to our map
			 m_mapBills.insert(std::make_pair(pBill->nID, pBill));

			 //// (j.gruber 2016-02-01 13:43) - PLID 68003 - check to see if our provider is already in the provider list and if not, add it
			 FilterIterator itProv;
			 itProv = m_mapProviders.find(pBill->strProvider);
			 if (itProv == m_mapProviders.end())
			 {
				 //add it
				 if (!pBill->strProvider.IsEmpty())
				 {
					m_mapProviders.insert(std::make_pair(pBill->strProvider, pBill->strProvider));
				 }
			 }


			 // (j.gruber 2016-02-01 13:43) - PLID 68004 - check to see if our location is already in the location list and if not, add it
			 FilterIterator itLocation;
			 itLocation = m_mapLocations.find(pBill->strLocation);
			 if (itLocation == m_mapLocations.end())
			 {
				 //add it
				 if (!pBill->strLocation.IsEmpty())
				 {
					m_mapLocations.insert(std::make_pair(pBill->strLocation, pBill->strLocation));
				 }
			 }
		 }
		 
		// (j.gruber 2016-02-01 13:44) - PLID 68000 - reload the main datalist
		ReloadBillDataList();

		//should we refresh the filter lists?
		if (bReloadFilterDataLists)
		{
			ReloadFilterDataLists();
		}

	}NxCatchAll(__FUNCTION__);

}

// (j.gruber 2016-02-01 13:44) - PLID 68003 - Create a provider filter on the HL7 visit and make it work
void CHL7ToBeBilledDlg::ReloadFilterDataLists()
{
	try
	{
		ReloadProviderDataList();

		ReloadLocationDataList();


	}NxCatchAll(__FUNCTION__)
}

// (j.gruber 2016-02-01 13:44) - PLID 68000 - helper function that adds one row from the map to the datalist
void CHL7ToBeBilledDlg::AddBillRow(HL7BillPtr pBill)
{
	NXDATALIST2Lib::IRowSettingsPtr pRow = m_pBillList->GetNewRow();
	pRow->PutValue(blcID, pBill->nID);
	pRow->PutValue(blcGroupID, pBill->nGroupID);
	pRow->PutValue(blcGroupName, _bstr_t(pBill->strGroupName));
	pRow->PutValue(blcPersonID, pBill->nPersonID);
	pRow->PutValue(blcPatientName, _bstr_t(pBill->strPatientName));
	pRow->PutValue(blcDescription, _bstr_t(pBill->strDescription));
	pRow->PutValue(blcServiceDate, _variant_t(pBill->dtServiceDate, VT_DATE));
	pRow->PutValue(blcCharges, _variant_t(pBill->strCharges));
	pRow->PutValue(blcDxCodes, _variant_t(pBill->strDxCodes));
	pRow->PutValue(blcProvider, _bstr_t(pBill->strProvider));
	pRow->PutValue(blcLocation, _bstr_t(pBill->strLocation));
	pRow->PutValue(blcBillVisit, _bstr_t("Bill this Visit"));

	m_pBillList->AddRowSorted(pRow, NULL);

}

// (j.gruber 2016-02-01 13:44) - PLID 68000 - function that reloads the bill from map
void CHL7ToBeBilledDlg::ReloadBillDataList()
{
	try
	{
		//first clear
		m_pBillList->Clear();

		//add all providers to the list first
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pBillList->GetNewRow();
		
		//iterate through our Bill map and add rows to the datalist
		HL7BillIterator itBill;
		for (itBill = m_mapBills.begin(); itBill != m_mapBills.end(); itBill++)
		{
			AddBillRow(itBill->second);
		}

	}NxCatchAll(__FUNCTION__)
}


// (j.gruber 2016-01-28 11:44) - PLID 68003 - Create a provider filter on the HL7 visit and make it work
void CHL7ToBeBilledDlg::ReloadProviderDataList()
{
	try
	{
		//get what was previous saved
		CString strCurSel = GetRemotePropertyText("HL7toBeBilledProviderFilter", ALL_PROVIDERS, 0, GetCurrentUserName());
		

		//first clear
		m_pProviderList->Clear();

		//add all providers to the list first
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProviderList->GetNewRow();
		pRow->PutValue(plcName, _variant_t(ALL_PROVIDERS));
		pRow->PutValue(plcOrder, 1);
		m_pProviderList->AddRowAtEnd(pRow, NULL);
		
		//iterate through our provider list and add rows to the datalist
		FilterIterator itProv;
		for (itProv = m_mapProviders.begin(); itProv != m_mapProviders.end(); itProv++)
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pProviderList->GetNewRow();
			pRow->PutValue(plcName, (bstr_t)itProv->second);
			pRow->PutValue(plcOrder, 10);
			m_pProviderList->AddRowSorted(pRow, NULL);
		}		
		
		//set it back to what it was
		NXDATALIST2Lib::IRowSettingsPtr pSel = m_pProviderList->FindByColumn(plcName, _variant_t(strCurSel), NULL, TRUE);
		if (pSel)
		{
			SelChosenProviderListFilter(pSel);
		}
		else 
		{
			// we've got a NULL current selection, possibly because something was formerly not mapped but is now mapped, so just set the selection
			// to all providers
			pSel = m_pProviderList->FindByColumn(plcName, _variant_t(ALL_PROVIDERS), NULL, TRUE);
			SelChosenProviderListFilter(pSel);
		}
		
		
	}NxCatchAll(__FUNCTION__)
}

// (j.gruber 2016-01-28 11:45) - PLID 68004 - Add a location filter to the HL7 visits dialog
void CHL7ToBeBilledDlg::ReloadLocationDataList()
{
	try
	{
		//check to see if there is anything saved
		CString strCurSel = GetRemotePropertyText("HL7toBeBilledLocationFilter", ALL_LOCATIONS, 0, GetCurrentUserName());
		
		//first clear
		m_pLocationList->Clear();

		//add all locations to the list first
		NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLocationList->GetNewRow();
		pRow->PutValue(llcName, _variant_t(ALL_LOCATIONS));
		pRow->PutValue(llcOrder, 1);
		m_pLocationList->AddRowAtEnd(pRow, NULL);

		//iterate through our provider list and add rows to the datalist
		FilterIterator itLocation;
		for (itLocation = m_mapLocations.begin(); itLocation != m_mapLocations.end(); itLocation++)
		{
			NXDATALIST2Lib::IRowSettingsPtr pRow = m_pLocationList->GetNewRow();
			pRow->PutValue(llcName, (bstr_t)itLocation->second);
			pRow->PutValue(llcOrder, 10);
			m_pLocationList->AddRowSorted(pRow, NULL);
		}

		//set it back to what it was
		NXDATALIST2Lib::IRowSettingsPtr pSel = m_pLocationList->FindByColumn(llcName, _variant_t(strCurSel), NULL, TRUE);
		if (pSel)
		{
			SelChosenLocationListFilter(pSel);
		}
		else
		{
			// we've got a NULL current selection, possibly because something was formerly not mapped but is now mapped, so just set the selection
			// to all locations
			pSel = m_pLocationList->FindByColumn(llcName, _variant_t(ALL_LOCATIONS), NULL, TRUE);
			SelChosenLocationListFilter(pSel);
		}
		
	}NxCatchAll(__FUNCTION__)
}

// (j.gruber 2016-01-28 11:44) - PLID 68003 - Create a provider filter on the HL7 visit and make it work
// (j.gruber 2016-01-28 11:45) - PLID 68004 - Add a location filter to the HL7 visits dialog
void CHL7ToBeBilledDlg::FilterLists()
{
	try
	{
		//clear the bill data list
		m_pBillList->Clear();

		// (j.gruber 2016-02-01 12:17) - PLID 68120 - check to see if filters are enabled
		CString strLocationName = ALL_LOCATIONS;
		CString strProviderName = ALL_PROVIDERS;

		if (IsDlgButtonChecked(IDC_ENABLE_FILTERS))
		{
			//if the filters are enabled, then check the values

			NXDATALIST2Lib::IRowSettingsPtr pProvRow = m_pProviderList->CurSel;
			NXDATALIST2Lib::IRowSettingsPtr pLocRow = m_pLocationList->CurSel;

			if (pProvRow && pLocRow)
			{

				strLocationName = VarString(pLocRow->GetValue(llcName), ALL_LOCATIONS);
				strProviderName = VarString(pProvRow->GetValue(plcName), ALL_PROVIDERS);

			}
		}

		//are we only filtering providers?
		if (strLocationName == ALL_LOCATIONS && strProviderName != ALL_PROVIDERS)
		{

			//now loop through our map and put back in anything that is for this location
			HL7BillIterator itBill;
			for (itBill = m_mapBills.begin(); itBill != m_mapBills.end(); itBill++)
			{
				HL7BillPtr pBill = itBill->second;
				if (pBill->strProvider == strProviderName)
				{
					AddBillRow(pBill);
				}
			}
		}
		//what about all providers but only some locations
		else if (strLocationName != ALL_LOCATIONS && strProviderName == ALL_PROVIDERS)
		{
			//now loop through our map and put back in anything that is for this location
			HL7BillIterator itBill;
			for (itBill = m_mapBills.begin(); itBill != m_mapBills.end(); itBill++)
			{
				HL7BillPtr pBill = itBill->second;
				if (pBill->strLocation == strLocationName)
				{
					AddBillRow(pBill);
				}
			}

		}
		//filters on both provider and location
		else if (strLocationName != ALL_LOCATIONS && strProviderName != ALL_PROVIDERS)
		{
			//now loop through our map and put back in anything that is for this location
			HL7BillIterator itBill;
			for (itBill = m_mapBills.begin(); itBill != m_mapBills.end(); itBill++)
			{
				HL7BillPtr pBill = itBill->second;
				if (pBill->strLocation == strLocationName && pBill->strProvider == strProviderName)
				{
					AddBillRow(pBill);
				}
			}
		}
		else {
			//add them all
			HL7BillIterator itBill;
			for (itBill = m_mapBills.begin(); itBill != m_mapBills.end(); itBill++)
			{
				HL7BillPtr pBill = itBill->second;
				AddBillRow(pBill);
			}

		}
		
	}NxCatchAll(__FUNCTION__)
}

// (j.gruber 2016-02-01 13:45) - PLID 68005 - Add a refresh button to the HL7 visits dialog
void CHL7ToBeBilledDlg::OnBnClickedHl7BillReload()
{
	try {
		ReloadMaps(TRUE);

	}NxCatchAll(__FUNCTION__);

}
BEGIN_EVENTSINK_MAP(CHL7ToBeBilledDlg, CNxDialog)
	ON_EVENT(CHL7ToBeBilledDlg, IDC_PROVIDER_LIST_FILTER, 16, CHL7ToBeBilledDlg::SelChosenProviderListFilter, VTS_DISPATCH)
	ON_EVENT(CHL7ToBeBilledDlg, IDC_LOCATION_LIST_FILTER, 16, CHL7ToBeBilledDlg::SelChosenLocationListFilter, VTS_DISPATCH)
	ON_EVENT(CHL7ToBeBilledDlg, IDC_HL7_BILLS, 19, CHL7ToBeBilledDlg::LeftClickHl7Bills, VTS_DISPATCH VTS_I2 VTS_I4 VTS_I4 VTS_I4)
	ON_EVENT(CHL7ToBeBilledDlg, IDC_PROVIDER_LIST_FILTER, 1 /* SelChanging */, CHL7ToBeBilledDlg::SelChangingProviderListFilter, VTS_DISPATCH VTS_PDISPATCH)
	ON_EVENT(CHL7ToBeBilledDlg, IDC_LOCATION_LIST_FILTER, 1 /* SelChanging */, CHL7ToBeBilledDlg::SelChangingLocationListFilter, VTS_DISPATCH VTS_PDISPATCH)
END_EVENTSINK_MAP()

// (j.gruber 2016-02-01 13:46) - PLID 68003 - Create a provider filter on the HL7 visit and make it work
void CHL7ToBeBilledDlg::SelChosenProviderListFilter(LPDISPATCH lpRow)
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow)
		{
			CString strProvider = VarString(pRow->GetValue(plcName));
			//check if we are doing all providers
			
				//filter the list
			FilterLists();		

			//store the selection
			SetRemotePropertyText("HL7ToBeBilledProviderFilter", strProvider, 0, GetCurrentUserName());
			}

	}NxCatchAll(__FUNCTION__)
}

// (j.gruber 2016-02-01 13:46) - PLID 68004 - Add a location filter to the HL7 visits dialog
void CHL7ToBeBilledDlg::SelChosenLocationListFilter(LPDISPATCH lpRow)
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow)
		{
			FilterLists();		

			CString strLocation = VarString(pRow->GetValue(llcName), ALL_LOCATIONS);

			//store the selection
			SetRemotePropertyText("HL7ToBeBilledLocationFilter", strLocation, 0, GetCurrentUserName());
		}

	}NxCatchAll(__FUNCTION__)
}

// (j.gruber 2016-02-01 13:46) - PLID 68008 - Pop out a menu that allows them to choose what responsibility the bill is going to
long CHL7ToBeBilledDlg::GetBillToResponsibility(long nPersonID, int x, int y)
{
	long nInsuredPartyID = -1;

	//pop out the menu
	ADODB::_RecordsetPtr rs = CreateParamRecordset("SELECT InsuredPartyT.PersonID, InsuranceCoT.Name, RespTypeT.TypeName, RespTypeT.ID AS RespTypeID "
		"FROM InsuredPartyT "
		"INNER JOIN InsuranceCoT ON InsuredPartyT.InsuranceCoID = InsuranceCoT.PersonID "
		"INNER JOIN RespTypeT ON InsuredPartyT.RespTypeID = RespTypeT.ID "
		"WHERE PatientID = {INT} AND RespTypeT.Priority <> -1 "
		"ORDER BY Coalesce(RespTypeT.CategoryPlacement,1000), RespTypeT.CategoryType, RespTypeT.Priority", nPersonID);
	if (!rs->eof) {

	
		CNxMenu mnu;
		mnu.m_hMenu = CreatePopupMenu();
	
	
		
		Nx::MenuCommandMap<long> menuCmds;

		long nIndex = 0;
		//add a line for patient
		mnu.InsertMenu(nIndex++, MF_BYPOSITION, menuCmds(-1), "For Patient Responsibility");

		//add a line for each responsibility
		while (!rs->eof) {

			long nInsPartyID = AdoFldLong(rs, "PersonID", -1);
			CString strInsCoName = AdoFldString(rs, "Name", "");
			CString strRespTypeName = AdoFldString(rs, "TypeName", "");
			CString strLabel;
			strLabel.Format("For %s (%s)", strInsCoName, strRespTypeName);

			mnu.InsertMenu(nIndex++, MF_BYPOSITION, menuCmds(nInsPartyID), strLabel);
			rs->MoveNext();
		}

		CPoint pt;
		GetCursorPos(&pt);

		nInsuredPartyID = mnu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD, pt.x, pt.y, this, NULL);
		mnu.DestroyMenu();


		
		if (nInsuredPartyID == 0) {
			return -2;
		}

		
		boost::optional<long> choice = menuCmds.Translate(nInsuredPartyID);
		if (!choice) {
			ASSERT(FALSE);
			return -2;
		}
		else {
			nInsuredPartyID = *choice;
		}
	}
	else {
		nInsuredPartyID = -1;
	}
	rs->Close();

	return nInsuredPartyID;

}

// (j.gruber 2016-02-01 13:47) - PLID 68006 - Be able to click on the "Bill this visit" column and bill the visit
void CHL7ToBeBilledDlg::LeftClickHl7Bills(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags)
{
	try
	{
		NXDATALIST2Lib::IRowSettingsPtr pRow(lpRow);
		if (pRow)
		{
			if (nCol == blcBillVisit)
			{
				//first check permissions
				if (!CheckCurrentUserPermissions(bioHL7Bills, sptDynamic0))
				{
					return;
				}
				
				long nPersonID = VarLong(pRow->GetValue(blcPersonID));
				long nID = VarLong(pRow->GetValue(blcID));
				long nInsuredPartyID = -1;
				if (nPersonID != -1)
				{
					nInsuredPartyID = GetBillToResponsibility(nPersonID, x, y);
				}

				if (nInsuredPartyID == -2)
				{
					//they clicked off the menu
					return;
				}
				CommitBill(nID, nInsuredPartyID);
			}
		}


	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2016-02-01 13:47) - PLID 68006 - Be able to click on the "Bill this visit" column and bill the visit
void CHL7ToBeBilledDlg::CommitBill(long nID, long nInsuredPartyID)
{
	try {

		//no warnings here

		//get our Bill from the map
		HL7BillIterator itBill;
		itBill = m_mapBills.find(nID);
		if (itBill != m_mapBills.end())
		{
			HL7BillPtr pBill = itBill->second;

			HL7Message Message;
			Message.nMessageID = pBill->nID;
			Message.strMessage = pBill->strMessage;
			Message.nHL7GroupID = pBill->nGroupID;
			Message.strHL7GroupName = pBill->strGroupName;
			Message.strPatientName = pBill->strPatientName;
			Message.dtInputDate = pBill->dtInputDate;

			// (r.gonet 05/01/2014) - PLID 61843 - We're beginning a new HL7 message transaction
			BeginNewHL7Transaction(GetRemoteData(), GetHL7SettingInt(Message.nHL7GroupID, "CurrentLoggingLevel"));

			ENotFoundResult nfr = nfrFailure;
			//TES 9/18/2008 - PLID 31414 - Renamed
			long nPersonID = GetPatientFromHL7Message(Message.strMessage, Message.nHL7GroupID, this, nfbSkip, &nfr);

			// (r.gonet 05/01/2014) - PLID 61843 - Store HL7 message ID so that the message may be pulled up when going over the HL7 logs.
			if (GetHL7Transaction()) {
				GetHL7Transaction()->SetHL7MessageID(Message.nMessageID);
			}

			if (GetHL7Transaction()) {
				GetHL7Transaction()->SetMessageType("DFT");
			}

			if (GetHL7Transaction()) {
				GetHL7Transaction()->SetEventType("P03");
			}

			BOOL bSuccessfullyCommitted = FALSE;
			CString strVersion;
			if (COMPONENT_FOUND != GetHL7MessageComponent(Message.strMessage, Message.nHL7GroupID, "MSH", 1, 12, 1, 1, strVersion)) {
				bSuccessfullyCommitted = FALSE;
			}

			bSuccessfullyCommitted = CreateHL7Bill(Message, this, strVersion, FALSE, nInsuredPartyID, BillFromType::VisitsToBeBilled);

			if (bSuccessfullyCommitted)
			{
				//find the row and remove it
				NXDATALIST2Lib::IRowSettingsPtr pRow;
				pRow = m_pBillList->FindByColumn(blcID, nID, NULL, false);
				if (pRow)
				{
					m_pBillList->RemoveRow(pRow);
				}
			}

		}

	}NxCatchAll(__FUNCTION__);

}

void CHL7ToBeBilledDlg::OnSize(UINT nType, int cx, int cy)
{
	try {
		CNxDialog::OnSize(nType, cx, cy);
	}NxCatchAll(__FUNCTION__);
}

// (j.gruber 2016-02-01 12:17) - PLID 68120 - As a biller, I must be able to enable filters for the Visits To Be Billed dialog.
void CHL7ToBeBilledDlg::OnBnClickedEnableFilters()
{
	try
	{
		BOOL bChecked = IsDlgButtonChecked(IDC_ENABLE_FILTERS);
		if (bChecked)
		{
			SetRemotePropertyInt("HL7ToBeBilledEnableFilters", 1, 0, GetCurrentUserName());
			m_pProviderList->Enabled = true;
			m_pLocationList->Enabled = true;
		}
		else {
			SetRemotePropertyInt("HL7ToBeBilledEnableFilters", 0, 0, GetCurrentUserName());
			m_pProviderList->Enabled = false;
			m_pLocationList->Enabled = false;
		}

		FilterLists();

	}NxCatchAll(__FUNCTION__);
}


void CHL7ToBeBilledDlg::SelChangingProviderListFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll(__FUNCTION__);
}

void CHL7ToBeBilledDlg::SelChangingLocationListFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel)
{
	try {
		if (*lppNewSel == NULL) {
			SafeSetCOMPointer(lppNewSel, lpOldSel);
		}
	}
	NxCatchAll(__FUNCTION__);
}

bool CHL7ToBeBilledDlg::MapHL7ProviderOrLocationField(HL7BillPtr pBill, boost::unordered_map<std::pair<long, CString>, CString> mappedHL7Fields, std::vector<std::tuple<CString, CString, CString>> vecProviderNames, bool bMapLocation)
{
	CString thirdPartyID = (bMapLocation ? 
		GetThirdPartyLocationIDFromMDIHL7Message(pBill->strMessage, pBill->nGroupID, GetHL7SettingBit(pBill->nGroupID, "EnableIntelleChart"))
		: GetThirdPartyProviderIDFromHL7Message(pBill->strMessage, pBill->nGroupID));
	auto it = mappedHL7Fields.find(std::make_pair(pBill->nGroupID, thirdPartyID));
	if (it != mappedHL7Fields.end())
	{
		// the HL7 Group ID/third party code was found, so now set the location or provider name to the mapped location/provider name that corresponds to that 
		// Group ID/third party code pair.
		if (bMapLocation)
		{
			pBill->strLocation = it->second;
		}
		else 
		{
			pBill->strProvider = it->second;
		}
		return true;
	}
	else
	{
		// a mapping for the provider/location by third party ID could not be found
		// if we're trying to map providers, then there's still a chance that we can map the provider to a provider in Practice via the provider name,
		// so try that
		if (!bMapLocation)
		{
			CString strBillFirst = GetProviderFirstNameFromHL7BillMessage(pBill->strMessage, pBill->nGroupID);
			CString strBillMiddle = GetProviderMiddleNameFromHL7BillMessage(pBill->strMessage, pBill->nGroupID);
			CString strBillLast = GetProviderLastNameFromHL7BillMessage(pBill->strMessage, pBill->nGroupID);
			CString strFoundProviderFullName = "";
			CString strFoundProviderFirstLastName = "";
			long nFullNameMatches = 0;
			long nFirstLastNameMatches = 0;

			// loop through all of the provider names in the vector of Practice provider name tuples, splitting out first, middle, and last from the tuples in the vector, and then
			// compare those names to the names pulled from the bill
			for (std::tuple<CString, CString, CString> namesTuple : vecProviderNames)
			{
				CString strPracticeFirst = std::get<0>(namesTuple);
				CString strPracticeMiddle = std::get<1>(namesTuple);
				CString strPracticeLast = std::get<2>(namesTuple);

				if (strBillFirst.CompareNoCase(strPracticeFirst) == 0 && !strPracticeMiddle.IsEmpty() && strBillMiddle.CompareNoCase(strPracticeMiddle) == 0 && strBillLast.CompareNoCase(strPracticeLast) == 0)
				{
					// the first, middle, and last name from the bill all match, so increase the match count and set the found provider name
					strFoundProviderFullName = strPracticeLast + ", " + strPracticeFirst + " " + strPracticeMiddle;
					nFullNameMatches++;
				}
				// the first, middle, and last name don't match, but it's possible that either the bill or the Practice provider just doesn't have a middle name,
				// so now try to map using just the first and last name
				else if (strBillFirst.CompareNoCase(strPracticeFirst) == 0 && strBillLast.CompareNoCase(strPracticeLast) == 0)
				{
					// the first and last names match, so increase the match count. Include the middle name in the found provider name, in case it exists in 
					// the database but not in the bill name
					strFoundProviderFirstLastName = strPracticeLast + ", " + strPracticeFirst + " " + strPracticeMiddle;
					nFirstLastNameMatches++;
				}
			}

			if (nFullNameMatches == 1)
			{
				// the provider on the bill was mapped by first, middle, and last name to exactly one provider, 
				pBill->strProvider = strFoundProviderFullName;
				return true;
			}
			// even if we've only got one match on first and last name, if we've also got matches on full name then this can't safely be considered mapped
			// because that means that there was at least one other provider with the same first and last name that also had a middle name, so we have no way of knowing
			// who should be mapped
			else if (nFirstLastNameMatches == 1 && nFullNameMatches == 0)
			{
				// the provider on the bill was mapped by first and last name to exactly one provider,
				pBill->strProvider = strFoundProviderFirstLastName;
				return true;
			}
			else
			{
				// couldn't map the provider by third party ID or name, so it's not mapped
				pBill->strProvider = FormatString("Not Mapped (%s)", pBill->strProvider.IsEmpty() ? "No Provider Provided" : pBill->strProvider);
			}
		}
		else
		{
			// there's no alternate mapping attempt for locations, so this location simply isn't mapped
			pBill->strLocation = FormatString("Not Mapped (%s)", pBill->strLocation.IsEmpty() ? "No Location Provided" : pBill->strLocation);
		}
		return false;
	}
}