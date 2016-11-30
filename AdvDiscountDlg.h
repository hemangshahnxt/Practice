#pragma once

// CAdvDiscountDlg dialog

class CAdvDiscountDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CAdvDiscountDlg)

public:
	// (j.gruber 2009-03-31 16:54) - PLID 33554 - added bisbill and bCheckDiscounts
	// (j.jones 2011-08-24 08:41) - PLID 44868 - added a map for original/void charges
	CAdvDiscountDlg(CMap<long, long, struct DiscountList*, DiscountList*> *mapDiscounts,
								 CMap<long, long, CString, CString> *mapChargeDescs,
								 CMap<long, long, BOOL, BOOL> *mapCorrectedCharges,
								 BOOL bIsBill,
								 CWnd* pParent =NULL);
	virtual ~CAdvDiscountDlg();

// Dialog Data
	enum { IDD = IDD_ADV_DISCOUNT_DLG };

protected:

	virtual BOOL OnInitDialog();
	CMap<long, long, DiscountList *, DiscountList*> *m_mapDiscounts;
	CMap<long, long, CString , CString > *m_mapChargeDescs;
	// (j.jones 2011-08-24 08:41) - PLID 44868 - added a map for original/void charges
	CMap<long, long, BOOL, BOOL> *m_mapCorrectedCharges;
	NXDATALIST2Lib::_DNxDataListPtr m_pCategoryList;
	NXDATALIST2Lib::_DNxDataListPtr m_pDiscountList;

	void LoadCategoryList();
	void LoadDiscountsForCategory(CString strID, CString strName);
	void AddRowToDatalist(long nLineID, struct stDiscount Disc);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	BOOL m_bIsBill;

	CNxIconButton m_btnOK;
	CNxIconButton m_btnCancel;
	CNxIconButton m_btnDelete;

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	DECLARE_EVENTSINK_MAP()
	void EditingStartingAdvDiscountDiscountList(LPDISPATCH lpRow, short nCol, VARIANT* pvarValue, BOOL* pbContinue);
	void EditingFinishedAdvDiscountDiscountList(LPDISPATCH lpRow, short nCol, const VARIANT& varOldValue, const VARIANT& varNewValue, BOOL bCommit);
	afx_msg void OnBnClickedAdvDiscountDeleteAll();
	void SelChosenAdvDiscountCategoryList(LPDISPATCH lpRow);
};