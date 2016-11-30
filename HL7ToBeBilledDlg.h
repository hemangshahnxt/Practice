#pragma once

#include <boost/unordered_map.hpp>
//(j.camacho 2016-01-27) PLID 68000 - Create HL7 Bill Dialog
// CHL7ToBeBilledDlg dialog

struct HL7Bill {

	HL7Bill()
	{
		nID = -1;
		nGroupID = -1;
		nPersonID = -1;
		strGroupName = "";
		dtServiceDate.SetStatus(COleDateTime::invalid);
		strPatientName = "";
		strDescription = "";
		strCharges = "";
		strDxCodes = "";
		strProvider = "";
		strLocation = "";
		strMessage = "";
		dtInputDate.SetStatus(COleDateTime::invalid);
		

	}

	long nID;
	long nGroupID;
	long nPersonID;
	CString strGroupName;
	COleDateTime dtServiceDate;
	CString strPatientName;
	CString strDescription;
	CString strCharges;
	CString strDxCodes;
	CString strProvider;
	CString strLocation;
	CString strMessage;
	COleDateTime dtInputDate;
};

typedef boost::shared_ptr<HL7Bill> HL7BillPtr;
typedef std::map<long, HL7BillPtr> HL7BillMap;
typedef HL7BillMap::iterator HL7BillIterator;

typedef std::map<CString, CString> FilterMap;
typedef FilterMap::iterator FilterIterator;


class CHL7ToBeBilledDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CHL7ToBeBilledDlg)

public:
	CHL7ToBeBilledDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CHL7ToBeBilledDlg();
	enum { IDD = IDD_HL7_TO_BE_BILLED_DLG };
	

protected:
	NXDATALIST2Lib::_DNxDataListPtr m_pBillList;
	NXDATALIST2Lib::_DNxDataListPtr m_pProviderList;
	NXDATALIST2Lib::_DNxDataListPtr m_pLocationList;
	HL7BillMap m_mapBills;
	FilterMap m_mapProviders;
	FilterMap m_mapLocations;

	LRESULT OnNcHitTest(CPoint point); //(j.camacho 2016-3-22) plid 68001

	// (j.gruber 2016-02-01 13:43) - PLID 68000 - main function that reloads our map from data
	void ReloadMaps(BOOL bReloadFilterDataLists);
	// (j.gruber 2016-01-28 11:44) - PLID 68003 - Create a provider filter on the HL7 visit and make it work
	// (j.gruber 2016-01-28 11:45) - PLID 68004 - Add a location filter to the HL7 visits dialog
	void ReloadFilterDataLists();

	// (j.gruber 2016-02-01 13:43) - PLID 68000 - reload main datalist from map
	void ReloadBillDataList();

	// (j.gruber 2016-02-01 13:43) - PLID 68000 - helper function adds one bill from map to list
	void AddBillRow(HL7BillPtr pBill);
	

	// (j.gruber 2016-01-28 11:44) - PLID 68003 - Create a provider filter on the HL7 visit and make it work
	// (j.gruber 2016-01-28 11:45) - PLID 68004 - Add a location filter to the HL7 visits dialog
	void FilterLists();
	void ReloadLocationDataList();
	void ReloadProviderDataList();

	/// <summary>
	/// Attempts to map the provided HL7 bill's location or provider fields to the fields that they are mapped to in Practice.
	/// </summary>
	/// <param name="pBill">The HL7 bill to attempt to map its location or provider field for.</param>
	/// <param name="mappedHL7Fields">Map of HL7 group IDs and third party IDs that map either a Practice location or provider name.</param>
	/// <param name="vecProviderNames">vector of all provider names split out into first, middle, and last values, to be optionally used if needed when attempting to map a provider.</param>
	/// <param name="bMapLocation">Indicates whether or not we are attempting to map a location. If not, then a provider mapping should be attempted.</param>
	/// <returns>True if the field was successfully mapped, false otherwise.</returns>
	bool MapHL7ProviderOrLocationField(HL7BillPtr pBill, boost::unordered_map<std::pair<long, CString>, CString> mappedHL7Fields, std::vector<std::tuple<CString, CString, CString>> vecProviderNames, bool bMapLocation);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	CNxIconButton m_btnClose, m_btnRefresh;

	DECLARE_MESSAGE_MAP()
public:

	afx_msg void OnCancel();
	afx_msg void OnBnClickedOk();
	// (j.gruber 2016-02-01 13:45) - PLID 68005 - Add a refresh button to the HL7 visits dialog
	afx_msg void OnBnClickedHl7BillReload();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	
	DECLARE_EVENTSINK_MAP()
	// (j.gruber 2016-01-28 11:44) - PLID 68003 - Create a provider filter on the HL7 visit and make it work
	// (j.gruber 2016-01-28 11:45) - PLID 68004 - Add a location filter to the HL7 visits dialog
	void SelChosenProviderListFilter(LPDISPATCH lpRow);
	void SelChosenLocationListFilter(LPDISPATCH lpRow);
	afx_msg void SelChangingProviderListFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	afx_msg void SelChangingLocationListFilter(LPDISPATCH lpOldSel, LPDISPATCH FAR* lppNewSel);
	// (j.gruber 2016-02-01 13:47) - PLID 68006 - Be able to click on the "Bill this visit" column and bill the visit
	void LeftClickHl7Bills(LPDISPATCH lpRow, short nCol, long x, long y, long nFlags);
	// (j.gruber 2016-02-01 13:46) - PLID 68008 - Pop out a menu that allows them to choose what responsibility the bill is going to
	long GetBillToResponsibility(long nPersonID, int x, int y);
	// (j.gruber 2016-02-01 13:46) - PLID 68008 - Pop out a menu that allows them to choose what responsibility the bill is going to
	void CommitBill(long nID, long nInsuredPartyID);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	// (j.gruber 2016-02-01 12:17) - PLID 68120 - As a biller, I must be able to enable filters for the Visits To Be Billed dialog.
	afx_msg void OnBnClickedEnableFilters();

};
