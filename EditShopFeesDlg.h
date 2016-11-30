#pragma once
#include "AdministratorRc.h"

// CEditShopFeesDlg dialog
// (j.gruber 2012-10-25 15:04) - PLID 53416 - create for

class CEditShopFeesDlg : public CNxDialog
{
	DECLARE_DYNAMIC(CEditShopFeesDlg)

public:
	CEditShopFeesDlg(long nServiceID, CString strServiceName, COleCurrency cyStandardFee, COLORREF color, CWnd* pParent = NULL);   // standard constructor
	virtual ~CEditShopFeesDlg();


	NXDATALIST2Lib::_DNxDataListPtr m_pAvailList;
	NXDATALIST2Lib::_DNxDataListPtr m_pSelectedList;

	CNxIconButton m_btnMoveOneLeft;
	CNxIconButton m_btnMoveOneRight;
	CNxIconButton m_btnMoveAllLeft;
	CNxIconButton m_btnMoveAllRight;
	CNxIconButton m_btnApply;
	CNxIconButton m_btnClose;	

// Dialog Data
	enum { IDD = IDD_EDIT_SHOP_FEE_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	long m_nServiceID;
	CString m_strServiceName;
	COLORREF m_color;
	CNxColor	m_bkgColor;
	COleCurrency m_cyStandardFee;
	void FillSelectedLocationIDs(CArray<long, long> *paryLocations);
	void Reload();
	
	void MoveOneLeft(NXDATALIST2Lib::IRowSettingsPtr pRowToMove);
	void MoveOneRight(NXDATALIST2Lib::IRowSettingsPtr pRowToMove);
	BOOL Validate();	


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedAllRight();
	afx_msg void OnBnClickedOneRight();
	afx_msg void OnBnClickedOneLeft();
	afx_msg void OnBnClickedAllLeft();
	afx_msg void OnBnClickedOk();
	DECLARE_EVENTSINK_MAP()
	void DblClickCellAvailLocationList(LPDISPATCH lpRow, short nColIndex);
	void DblClickCellSelectedLocationList(LPDISPATCH lpRow, short nColIndex);
	afx_msg void OnEnKillfocusShopFee();
};
