#pragma once

// CAdvPreferenceCardEditDlg dialog

// (j.jones 2009-08-27 09:51) - PLID 35283 - created

#include "AdministratorRc.h"

class CAdvPreferenceCardEditDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAdvPreferenceCardEditDlg)

public:
	CAdvPreferenceCardEditDlg(CWnd* pParent);   // standard constructor

// Dialog Data
	enum { IDD = IDD_ADV_PREFERENCE_CARD_EDIT_DLG };
	CNxIconButton	m_btnSelectOnePrefCard;
	CNxIconButton	m_btnUnselectOnePrefCard;
	CNxIconButton	m_btnUnselectAllPrefCards;
	CNxIconButton	m_btnSelectOneProduct;
	CNxIconButton	m_btnUnselectOneProduct;
	CNxIconButton	m_btnUnselectAllProducts;
	CNxIconButton	m_btnSelectOnePerson;
	CNxIconButton	m_btnUnselectOnePerson;
	CNxIconButton	m_btnUnselectAllPersons;
	CNxIconButton	m_btnSelectOneService;
	CNxIconButton	m_btnUnselectOneService;
	CNxIconButton	m_btnUnselectAllServices;
	CNxIconButton	m_btnApply;
	CNxIconButton	m_btnClose;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedPreferenceCardList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedPreferenceCardList;
	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedProductList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedProductList;
	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedPersonnelList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedPersonnelList;
	NXDATALIST2Lib::_DNxDataListPtr m_UnselectedServiceList;
	NXDATALIST2Lib::_DNxDataListPtr m_SelectedServiceList;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBtnSelectOnePrefCard();
	afx_msg void OnBnClickedBtnUnselectOnePrefCard();
	afx_msg void OnBnClickedBtnUnselectAllPrefCards();
	afx_msg void OnBnClickedBtnSelectOneProduct();
	afx_msg void OnBnClickedBtnUnselectOneProduct();
	afx_msg void OnBnClickedBtnUnselectAllProducts();
	afx_msg void OnBnClickedBtnSelectOnePerson();
	afx_msg void OnBnClickedBtnUnselectOnePerson();
	afx_msg void OnBnClickedBtnUnselectAllPersons();
	afx_msg void OnBnClickedBtnSelectOneService();
	afx_msg void OnBnClickedBtnUnselectOneService();
	afx_msg void OnBnClickedBtnUnselectAllServices();
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedBtnClosePcEdit();

	DECLARE_EVENTSINK_MAP()
	void OnDblClickCellUnselectedPreferenceCardList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedPreferenceCardList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellUnselectedInventoryList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedInventoryList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellUnselectedPersonList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedPersonList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellUnselectedServiceList(LPDISPATCH lpRow, short nColIndex);
	void OnDblClickCellSelectedServiceList(LPDISPATCH lpRow, short nColIndex);
};
