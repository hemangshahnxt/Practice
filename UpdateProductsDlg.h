#pragma once

// CUpdateProductsDlg dialog

// (j.jones 2010-01-11 10:27) - PLID 26786 - created

#include "InventoryRc.h"

class CUpdateProductsDlg : public CNxDialog
{

public:
	CUpdateProductsDlg(CWnd* pParent);   // standard constructor
	virtual ~CUpdateProductsDlg();

// Dialog Data
	enum { IDD = IDD_UPDATE_PRODUCTS_DLG };
	CNxIconButton	m_btnClose;
	CNxIconButton	m_btnApply;
	CNxIconButton	m_btnSelectOne;
	CNxIconButton	m_btnSelectAll;
	CNxIconButton	m_btnUnselectOne;
	CNxIconButton	m_btnUnselectAll;
	NxButton		m_checkUpdateCategory;
	NxButton		m_checkUpdateSupplier;
	NxButton		m_checkUpdateTaxable1;
	NxButton		m_checkUpdateTaxable2;
	NxButton		m_checkUpdateRevCode;
	NxButton		m_checkUpdateShopFee;
	NxButton		m_checkTaxable1;
	NxButton		m_checkTaxable2;
	CNxEdit			m_nxeditShopFee;
	NxButton		m_checkUpdateInsuranceCode;
	CNxEdit			m_nxeditInsuranceCode;
	// (j.jones 2010-01-13 10:12) - PLID 36847 - added per-location controls
	NxButton		m_checkUpdateBillable;
	NxButton		m_checkUpdateTrackable;
	NxButton		m_checkUpdateReorderPoint;
	NxButton		m_checkUpdateReorderQty;
	NxButton		m_checkUpdateUserResp;
	NxButton		m_checkBillable;
	NxButton		m_radioNotTrackable;
	NxButton		m_radioTrackOrders;
	NxButton		m_radioTrackQuantity;
	CNxEdit			m_nxeditReorderPoint;
	CNxEdit			m_nxeditReorderQty;
	NxButton		m_checkUpdateShopFeeLoc;	// (j.gruber 2012-10-29 13:47) - PLID 53241
	CNxEdit			m_nxeditShopFeeLoc; // (j.gruber 2012-10-29 13:47) - PLID 53241
	// (j.jones 2015-03-16 09:15) - PLID 64972 - added ability to select multiple categories
	CNxIconButton	m_btnSelectCategory;
	CNxIconButton	m_btnRemoveCategory;
	CNxEdit			m_editCategory;
	// (j.jones 2016-04-07 11:54) - NX-100076 - added ability to mass-update Remember Charge Provider
	NxButton		m_checkUpdateRememberChargeProvider;
	NxButton		m_checkRememberChargeProvider;

protected:
	long CheckConflict(long nIDBeingChecked); // (j.gruber 2012-10-29 13:13) - PLID 53241

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// (j.jones 2010-01-13 10:12) - PLID 36847 - the products need to reload by location
	void RefreshProductLists(long nLocationID);

	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedList;
	NXDATALIST2Lib::_DNxDataListPtr m_SupplierCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_RevCodeCombo;	
	// (j.jones 2010-01-13 10:12) - PLID 36847 - added per-location controls
	NXDATALIST2Lib::_DNxDataListPtr m_LocationCombo;
	NXDATALIST2Lib::_DNxDataListPtr m_UserCombo;

	// (j.jones 2015-03-16 09:15) - PLID 64972 - added ability to select multiple categories
	std::vector<long> m_aryCategoryIDs;
	long m_nDefaultCategoryID;

	DECLARE_MESSAGE_MAP()
	DECLARE_EVENTSINK_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnBtnApplyProductChanges();
	afx_msg void OnBtnSelectOne();
	afx_msg void OnBtnSelectAll();
	afx_msg void OnBtnUnselectOne();
	afx_msg void OnBtnUnselectAll();
	void OnDblClickCellUnselectedList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnBtnCloseProductUpdater();	
	afx_msg void OnCheckUpdateCategory();
	afx_msg void OnCheckUpdateSupplier();
	afx_msg void OnCheckUpdateTaxable1();
	afx_msg void OnCheckUpdateTaxable2();
	afx_msg void OnCheckUpdateRevCode();
	afx_msg void OnCheckUpdateShopFee();
	// (j.jones 2010-01-13 10:12) - PLID 36847 - added per-location controls
	void OnSelChosenLocationCombo(LPDISPATCH lpRow);
	afx_msg void OnCheckUpdateBillable();
	afx_msg void OnCheckUpdateTrackable();
	afx_msg void OnCheckUpdateReorderPoint();
	afx_msg void OnCheckUpdateReorderQty();
	afx_msg void OnCheckUpdateUserResp();
	afx_msg void OnBnClickedCheckUpdateShopFeeLoc(); // (j.gruber 2012-10-29 13:47) - PLID 53241
	// (j.jones 2015-03-16 09:15) - PLID 64972 - added ability to select multiple categories
	afx_msg void OnBtnSelectCategory();
	afx_msg void OnBtnRemoveCategory();
	// (j.jones 2016-04-07 11:54) - NX-100076 - added ability to mass-update Remember Charge Provider
	afx_msg void OnCheckUpdateRememberChargeProvider();
	afx_msg void OnCheckUpdateInsuranceCode();
};
