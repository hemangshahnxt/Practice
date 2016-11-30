#pragma once
#include "afxwin.h"

// (a.walling 2009-03-30 11:31) - PLID 33573 - Interface for SureScripts directories / pharmacy search

// CPharmacyDirectorySearchDlg dialog

class CPharmacyDirectorySearchDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CPharmacyDirectorySearchDlg)

public:
	CPharmacyDirectorySearchDlg(CWnd* pParent);   // standard constructor
	virtual ~CPharmacyDirectorySearchDlg();

// Dialog Data
	enum { IDD = IDD_PHARMACY_DIRECTORY_SEARCH_DLG };

	BOOL m_bMultiMode;
	long m_nSelectedID;
	long m_nTotalAdded;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	//TES 11/15/2012 - PLID 53807 - Took out AddNewPharmacy, all pharmacies are now added at once.
	
	// analagous to how the importer does things
	// (a.walling 2009-04-14 17:21) - PLID 33951 - FormatPhoneForImport Moved to SureScriptsPractice namespace

	CNxColor m_bkgColor;
	NXDATALIST2Lib::_DNxDataListPtr m_pList;
	// (r.gonet 2016-02-25 16:14) - PLID 67961 - Separate list containing just the mail order pharmacies. Added because there is a requirement that we
	// never filter on the address fields for mail order pharmacies. And with one search results list, these mail order pharmacies from around the country
	// were cluttering it significantly, when all the user wanted was some pharmacy down the road. Note that this does not limit the mail order pharmacies
	// from appearing in the main search results list if they actually match the matching criteria.
	NXDATALIST2Lib::_DNxDataListPtr m_pMailOrderPharmaciesList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSpecialties; //TES 2/8/2013 - PLID 55085
	// (j.jones 2016-02-29 10:51) - PLID 67986 - reordered the columns
	enum EPharmacySearchColumns {
		lcStoreName = 0,
		lcAddress,
		lcCity,
		lcState,
		lcZip,
		lcPhone,
		lcFax,
		lcCrossStreet,
		lcEmail,
		lcPhoneAlt,
		lcPhoneAlt2,		
		lcNPI,
		lcNCPDPID,
		lcStoreNumber,
		lcServices,
		lcColor,
		lcLocationID,
		lcSpecialty, //TES 2/8/2013 - PLID 55085
	};

	//TES 2/8/2013 - PLID 55085
	enum SpecialtyColumns {
		scSpecialty = 0,
	};

	CString GetWhereClause(bool bMailOrderOnly) throw(...);

	void LoadSettings() throw(...);
	void SaveSettings() throw(...);

	DECLARE_MESSAGE_MAP()
public:
	CNxIconButton m_nxibAdd;
	CNxIconButton m_nxibClose;
	afx_msg void OnBnClickedAdd();
	afx_msg void OnBnClickedCancel();
	CNxStatic m_nxsNCPDPID;
	CNxStatic m_nxsStoreName;
	CNxStatic m_nxsCity;
	CNxStatic m_nxsState;
	CNxStatic m_nxsZip;
	CNxStatic m_nxsPhone;
	CNxEdit m_nxeditNCPDPID;
	CNxEdit m_nxeditStoreName;
	CNxEdit m_nxeditCity;
	CNxEdit m_nxeditState;
	CNxEdit m_nxeditZip;
	CNxEdit m_nxeditPhone;
	CNxIconButton m_nxibRefresh;
	CNxStatic m_nxsLastUpdated;
	NxButton m_nxbGroupBox;
	CNxEdit m_nxeditAddress;//TES 1/22/2013 - PLID 54723
	// (j.jones 2016-02-23 14:16) - PLID 67986 - added support for cross streets
	CNxEdit m_nxeditCrossStreet;

	afx_msg void OnBnClickedBtnRefresh();
	afx_msg void OnDestroy();
	DECLARE_EVENTSINK_MAP()
	void DblClickCellList(LPDISPATCH lpRow, short nColIndex);
	void CurSelWasSetList();
	void CurSelWasSetMailOrderPharmaciesList();
	void DblClickCellMailOrderPharmaciesList(LPDISPATCH lpRow, short nColIndex);
};
